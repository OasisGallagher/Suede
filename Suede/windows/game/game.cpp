#include <QWidget>
#include "ui_suede.h"
#include "windows/controls/canvas.h"

#include "game.h"
#include "font.h"
#include "mesh.h"
#include "light.h"
#include "time2.h"
#include "world.h"
#include "camera.h"
#include "shader.h"
#include "engine.h"
#include "gizmos.h"
#include "texture.h"
#include "projector.h"
#include "variables.h"
#include "tagmanager.h"
#include "particlesystem.h"

#include "tools/math2.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/cameracontroller.h"

#define ROOM
#define SKYBOX
//#define PROJECTOR
//#define BEAR
//#define BEAR_X_RAY
//#define POST_EFFECTS
//#define MAN
//#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED
//#define DEFERRED_RENDERING

static Game* gameInstance;

Game* Game::get() {
	return gameInstance;
}

Game::Game(QWidget* parent) : QDockWidget(parent), initialized_(false) {
	gameInstance = this;
	controller_ = new CameraController;
}

Game::~Game() {
	delete grayscale_;
	delete inversion_;
	delete controller_;
	killTimer(updateTimer_);
	gameInstance = nullptr;
}

void Game::init(Ui::Suede* ui) {
	ChildWindow::init(ui);
	updateTimer_ = startTimer(10, Qt::PreciseTimer);

	connect(Hierarchy::get(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));
}

void Game::awake() {
	grayscale_ = new Grayscale;
	inversion_ = new Inversion;

	createScene();
	update();
}

void Game::OnDrawGizmos() {
	foreach (Entity entity, selected_) {
		if (entity->GetTransform()->GetPosition() == glm::vec3(0)) {
			continue;
		}

		const Bounds& bounds = entity->GetBounds();
		Gizmos::DrawCuboid(bounds.center, bounds.size);
	}
}

void Game::start() {
}

void Game::update() {
	ui_->canvas->update();
}

void Game::wheelEvent(QWheelEvent* event) {
	controller_->onMouseWheel(event->delta());
}

void Game::mousePressEvent(QMouseEvent *event) {
	controller_->onMousePress(event->button(), event->pos());
}

void Game::mouseReleaseEvent(QMouseEvent* event) {
	controller_->onMouseRelease(event->button());
}

void Game::mouseMoveEvent(QMouseEvent *event) {
	controller_->onMouseMove(event->pos());
}

void Game::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	}
}

void Game::resizeEvent(QResizeEvent* event) {
	controller_->onResize(event->size());
}

void Game::timerEvent(QTimerEvent *event) {
	if (event->timerId() != updateTimer_) {
		return;
	}

	if (Time::GetFrameCount() == 1) {
		start();
	}

	update();
}

void Game::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
	selected_ = selected;
}

uint roomEntityID;

void Game::createScene() {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(0.15f));
	DirectionalLight light = NewDirectionalLight();
	light->SetName("light");
	light->SetColor(glm::vec3(0.7f));

	Camera camera = NewCamera();
	WorldInstance()->SetMainCamera(camera);

	camera->AddGizmosPainter(this);

	camera->SetName("camera");
	controller_->setCamera(camera->GetTransform());

#ifdef PROJECTOR
	Projector projector = NewProjector();
	projector->SetPerspective(false);
	projector->SetOrthographicSize(5);
 	//projector->SetFieldOfView(Math::Radians(9));
	projector->GetTransform()->SetPosition(glm::vec3(0, 25, 0));

	Texture2D texture = NewTexture2D();
	texture->Load("textures/brick_diffuse.jpg");
	projector->SetTexture(texture);
#endif // PROJECTOR

	light->GetTransform()->SetParent(camera->GetTransform());

#ifdef DEFERRED_RENDERING
	camera->SetRenderPath(RenderPathDeferred);
#endif

	camera->SetFarClipPlane(10000.f);
	camera->GetTransform()->SetPosition(glm::vec3(0, 25, 0));

#ifdef POST_EFFECTS
	//camera->AddImageEffect(inversion_);
	camera->AddImageEffect(grayscale_);
#endif

#ifdef SKYBOX
	camera->SetClearType(ClearTypeSkybox);

	Material skybox = NewMaterial();
	skybox->SetShader(Resources::FindShader("buildin/shaders/skybox"));

	TextureCube cube = NewTextureCube();

	std::string faces[] = {
		"textures/lake_skybox/right.jpg",
		"textures/lake_skybox/left.jpg",
		"textures/lake_skybox/top.jpg",
		"textures/lake_skybox/bottom.jpg",
		"textures/lake_skybox/back.jpg",
		"textures/lake_skybox/front.jpg",
	};

	cube->Load(faces);
	skybox->SetTexture(Variables::mainTexture, cube);
	skybox->SetColor4(Variables::mainColor, glm::vec4(1));
	WorldInstance()->GetEnvironment()->SetSkybox(skybox);

#else
	camera->SetClearType(ClearTypeColor);
	camera->SetClearColor(glm::vec3(0, 0, 0.1f));
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = NewRenderTexture();
	renderTexture->Load(RenderTextureFormatRgba, ui_->canvas->width(), ui_->canvas->height());
	camera->SetRenderTexture(renderTexture);
#endif

#ifdef PARTICLE_SYSTEM
	ParticleSystem particleSystem = NewParticleSystem();
	particleSystem->GetTransform()->SetPosition(glm::vec3(-30, 20, -50));

	SphereParticleEmitter emitter = NewSphereParticleEmitter();
	emitter->SetRadius(5);
	emitter->SetRate(200);
	emitter->SetStartColor(glm::vec4(1, 1, 1, 0.5f));
	emitter->SetStartDuration(2);
	emitter->SetStartSize(1);
	emitter->SetStartVelocity(glm::vec3(0, 1, 0));
	ParticleBurst burst = { 4, 3, 20 };
	particleSystem->SetEmitter(emitter);

	ParticleAnimator animator = NewParticleAnimator();
	animator->SetGravityScale(0.2f);
	particleSystem->SetParticleAnimator(animator);

	particleSystem->SetMaxParticles(1000);
	particleSystem->SetDuration(5);
	particleSystem->SetLooping(true);
#endif

#if defined(FONT)
	Entity fentity = NewEntity();
	fentity->GetTransform()->SetPosition(glm::vec3(-10, 20, -20));
	fentity->GetTransform()->SetEulerAngles(glm::vec3(0, 0, 0));

	//Entity fentity2 = NewEntity();
	//fentity2->SetPosition(glm::vec3(-10, 30, -20));

	Font font = NewFont();
	font->Load("fonts/ms_yh.ttf", 12);

	TextMesh mesh = NewTextMesh();
	mesh->SetFont(font);
	mesh->SetText("ab");
	
	mesh->SetFontSize(12);
	fentity->SetMesh(mesh);
	//fentity2->SetMesh(mesh);

	Renderer renderer = NewMeshRenderer();
	Material fontMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial->SetColor4(Variables::mainColor, glm::vec4(1, 0, 0, 1));

	/*
	Renderer renderer2 = NewMeshRenderer();
	Material fontMaterial2 = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial2->SetColor(Variables::mainColor, glm::vec4(1, 1, 0, 1));
	*/

	renderer->AddMaterial(fontMaterial);
	//renderer2->AddMaterial(fontMaterial2);

	fentity->SetRenderer(renderer);
	//fentity2->SetRenderer(renderer2);
#endif

#ifdef ROOM
	Entity room = WorldInstance()->Import("models/geom.fbx");
// 	room->GetTransform()->SetPosition(glm::vec3(0, 25, -65));
// 	room->GetTransform()->SetEulerAngles(glm::vec3(30, 60, 0));
	//room->GetTransform()->SetScale(glm::vec3(0.01f));
	roomEntityID = room->GetInstanceID();
#endif

#ifdef BEAR
	Entity bear = WorldInstance()->Import("models/teddy_bear.fbx");
	bear->GetTransform()->SetPosition(glm::vec3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Resources::FindShader("shaders/xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef MAN
	Entity man = WorldInstance()->Import("models/boblampclean.md5mesh");
	man->GetTransform()->SetPosition(glm::vec3(0, 0, -70));
	man->GetTransform()->SetEulerAngles(glm::vec3(270, 180, 180));
	if (man) {
		//entity->SetParent(camera);

		Animation animation = man->GetAnimation();
		if (animation) {
			animation->SetWrapMode(AnimationWrapModePingPong);
			animation->Play("");
		}
	}
#endif
}
