#include <QWidget>
#include <glm/gtc/matrix_transform.hpp>

#include "ui_suede.h"
#include "windows/status/status.h"
#include "windows/controls/canvas.h"

#include "game.h"
#include "font.h"
#include "mesh.h"
#include "light.h"
#include "time2.h"
#include "world.h"
#include "screen.h"
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
//#define SKYBOX
//#define PROJECTOR
//#define PROJECTOR_ORTHOGRAPHIC
//#define BEAR
//#define BEAR_X_RAY
//#define IMAGE_EFFECTS
//#define MAN
//#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED
//#define DEFERRED_RENDERING

static const char* manFbxPath = "models/boblampclean.md5mesh";
static const char* roomFbxPath = "models/room.obj";// .fbx";

static Game* gameInstance;

Game* Game::get() {
	return gameInstance;
}

Game::Game(QWidget* parent) : QDockWidget(parent) {
	gameInstance = this;
	controller_ = new CameraController;
	Engine::AddFrameEventListener(this);
}

Game::~Game() {
	Engine::RemoveFrameEventListener(this);

	delete grayscale_;
	delete inversion_;
	delete controller_;

	killTimer(updateTimer_);

	gameInstance = nullptr;
}

void Game::init(Ui::Suede* ui) {
	ChildWindow::init(ui);

	updateTimer_ = startTimer(10, Qt::PreciseTimer);

	connect(Hierarchy::get(), SIGNAL(focusEntity(Entity)), this, SLOT(onFocusEntityBounds(Entity)));
	connect(Hierarchy::get(), SIGNAL(selectionChanged(const QList<Entity>&, const QList<Entity>&)),
		this, SLOT(onSelectionChanged(const QList<Entity>&, const QList<Entity>&)));
}

void Game::awake() {
	grayscale_ = new Grayscale;
	inversion_ = new Inversion;

	loadSceneStart_ = Time::GetRealTimeSinceStartup();
	createScene();
	update();
}

void Game::OnDrawGizmos() {
	int i = 0;
	glm::vec3 colors[] = { glm::vec3(0, 1, 0), glm::vec3(1, 0, 0) };
	glm::vec3 oldColor = Gizmos::GetColor();
	foreach(Entity entity, selection_) {
		if (!entity->GetActive()) {
			continue;
		}

		const Bounds& bounds = entity->GetBounds();
		if (!bounds.IsEmpty()) {
			Gizmos::SetColor(colors[i % CountOf(colors)]);
			Gizmos::DrawCuboid(bounds.center, bounds.size);
			++i;
		}
	}

	Gizmos::SetColor(oldColor);
}

void Game::OnEntityImported(Entity root, const std::string& path) {
	if (path == manFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 0, -70));
		root->GetTransform()->SetEulerAngles(glm::vec3(270, 180, 180));
		root->GetTransform()->SetScale(glm::vec3(0.2f));
		//entity->SetParent(camera);

		Animation animation = root->GetAnimation();
		if (animation) {
			animation->SetWrapMode(AnimationWrapModePingPong);
			animation->Play("");
		}
	}

	if (path == roomFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 25, -65));
		root->GetTransform()->SetEulerAngles(glm::vec3(30, 60, 0));
		if (path.find("house") != std::string::npos) {
			root->GetTransform()->SetScale(glm::vec3(0.01f));
		}

		float delta = Time::GetRealTimeSinceStartup() - loadSceneStart_;
		Status::get()->showMessage(QString("%1 loaded in %2 seconds").arg(path.c_str()).arg(QString::number(delta, 'g', 2)), 2000);
	}
}

void Game::OnFrameLeave() {
//	Graphics::Blit(targetTexture_, nullptr);
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

void Game::updateSelection(QList<Entity>& container, const QList<Entity>& selected, const QList<Entity>& deselected) {
	foreach(Entity entity, selected) {
		if (container.indexOf(entity) < 0) {
			container.push_back(entity);
		}
	}

	foreach(Entity entity, deselected) {
		container.removeOne(entity);
	}
}

void Game::onFocusEntityBounds(Entity entity) {
	Transform trans = entity->GetTransform();
	Transform camera = WorldInstance()->GetMainCamera()->GetTransform();
	glm::vec3 position = entity->GetBounds().center;
	glm::vec3 p = position - trans->GetForward() * calculateCameraDistanceFitsBounds(WorldInstance()->GetMainCamera(), entity);
	camera->SetPosition(p);

	glm::vec3 up(0, 1, 0);
	glm::vec3 forward = glm::normalize(position - camera->GetPosition());
	glm::vec3 right = glm::cross(up, forward);
	glm::quat q(glm::mat3(right, up, forward));
	camera->SetRotation(glm::normalize(q));
}

void Game::onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected) {
	updateSelection(selection_, selected, deselected);
}

float Game::calculateCameraDistanceFitsBounds(Camera camera, Entity entity) {
	const Bounds& b = entity->GetBounds();
	float f = tanf(camera->GetFieldOfView() / 2.f);
	float dy = 2 * b.size.y / f;
	float dx = 2 * b.size.x / (f* camera->GetAspect());
	return Math::Clamp(qMax(dx, dy), camera->GetNearClipPlane() + b.size.z * 2, camera->GetFarClipPlane() - b.size.z * 2);
}

uint roomEntityID;

void Game::createScene() {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(0.15f));
	DirectionalLight light = NewDirectionalLight();
	light->SetName("light");
	light->SetColor(glm::vec3(0.7f));
	light->GetTransform()->SetParent(WorldInstance()->GetRootTransform());

	//targetTexture_ = NewRenderTexture();
	//targetTexture_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());

	Camera camera = NewCamera();
	WorldInstance()->SetMainCamera(camera);
	camera->AddGizmosPainter(this);
	camera->GetTransform()->SetParent(WorldInstance()->GetRootTransform());

	camera->SetName("camera");
	controller_->setCamera(camera->GetTransform());

#ifdef PROJECTOR
	Projector projector = NewProjector();
	
#ifdef PROJECTOR_ORTHOGRAPHIC
	projector->SetPerspective(false);
	projector->SetOrthographicSize(5);
#else
	projector->SetFieldOfView(Math::Radians(9.f));
#endif

	projector->GetTransform()->SetPosition(glm::vec3(0, 25, 0));

	Texture2D texture = NewTexture2D();
	texture->Load("textures/brick_diffuse.jpg");
	projector->SetTexture(texture);
#endif // PROJECTOR

//	light->GetTransform()->SetParent(camera->GetTransform());

#ifdef DEFERRED_RENDERING
	camera->SetRenderPath(RenderPathDeferred);
#endif

	camera->SetFarClipPlane(10000.f);
	camera->GetTransform()->SetPosition(glm::vec3(0, 25, 0));
	camera->SetDepthTextureMode(DepthTextureModeDepth);
	/*camera->SetRect(Rect(0.f, 0.f, 0.5f, 0.5f));*/
	//camera->SetActiveSelf(false);
	//camera->SetTargetTexture(targetTexture_);
	
	//Camera camera2 = NewCamera();
	//camera2->SetFarClipPlane(10000.f);
	//camera2->GetTransform()->SetPosition(glm::vec3(0, 25, 0));
	//camera2->SetDepthTextureMode(DepthTextureModeDepth);

	//camera2->SetRect(Rect(0.5f, 0.5f, 0.5f, 0.5f));
	////camera2->SetTargetTexture(targetTexture_);

	//camera2->SetClearColor(glm::vec3(0.1f, 0, 0.1f));
	//camera2->SetDepth(-1);
	//camera2->SetName("MultiCameraTest");

	light->GetTransform()->SetPosition(glm::vec3(0, 25, 0));

#ifdef IMAGE_EFFECTS
	camera->AddImageEffect(grayscale_);
	//camera->AddImageEffect(inversion_);
#endif

#ifdef SKYBOX
	camera->SetClearType(ClearTypeSkybox);

	Material skybox = NewMaterial();
	skybox->SetShader(Resources::FindShader("builtin/skybox"));

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
	camera->SetClearColor(glm::vec3(0, 0.1f, 0.1f));
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = NewRenderTexture();
	renderTexture->Load(RenderTextureFormatRgba, ui_->canvas->width(), ui_->canvas->height());
	camera->SetRenderTexture(renderTexture);
#endif

#ifdef PARTICLE_SYSTEM
	Entity entity = NewEntity();
	ParticleSystem particleSystem = NewParticleSystem();
	entity->SetParticleSystem(particleSystem);
	entity->GetTransform()->SetPosition(glm::vec3(-30, 20, -50));

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
	Font font = NewFont();
	font->Load("fonts/ms_yh.ttf", 12);

	Entity redText = NewEntity();
	redText->GetTransform()->SetPosition(glm::vec3(-10, 20, -20));

	Entity blueText = NewEntity();
	blueText->GetTransform()->SetPosition(glm::vec3(-10, 30, -20));

	TextMesh redMesh = NewTextMesh();
	redMesh->SetFont(font);
	redMesh->SetText("落霞与孤鹜齐飞");
	redMesh->SetFontSize(12);

	TextMesh blueMesh = NewTextMesh();
	blueMesh->SetFont(font);
	blueMesh->SetText("秋水共长天一色");
	blueMesh->SetFontSize(12);

	redText->SetMesh(redMesh);
	blueText->SetMesh(blueMesh);

	Renderer redRenderer = NewMeshRenderer();
	Material redMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	redMaterial->SetColor4(Variables::mainColor, glm::vec4(1, 0, 0, 1));
	redRenderer->AddMaterial(redMaterial);

	Renderer blueRenderer = NewMeshRenderer();
	Material blueMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	blueMaterial->SetColor4(Variables::mainColor, glm::vec4(0, 0, 1, 1));
	blueRenderer->AddMaterial(blueMaterial);

	redText->SetRenderer(redRenderer);
	blueText->SetRenderer(blueRenderer);

#endif

#ifdef ROOM
	Entity room = WorldInstance()->Import(roomFbxPath, this);
	roomEntityID = room->GetInstanceID();
	Status::get()->showMessage("Loading models/house.fbx...", 0);
#endif

#ifdef BEAR
	Entity bear = WorldInstance()->Import("models/teddy_bear.fbx");
	bear->GetTransform()->SetPosition(glm::vec3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Resources::FindShader("xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef MAN
	Entity man = WorldInstance()->Import(manFbxPath, this);
#endif
}
