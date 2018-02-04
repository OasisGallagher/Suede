#include <QWidget>
#include "canvas.h"
#include "ui_suede.h"

#include "tags.h"
#include "game.h"
#include "font.h"
#include "mesh.h"
#include "light.h"
#include "time2.h"
#include "world.h"
#include "skybox.h"
#include "camera.h"
#include "shader.h"
#include "skybox.h"
#include "engine.h"
#include "texture.h"
#include "variables.h"
#include "tagmanager.h"
#include "particlesystem.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/cameracontroller.h"

#define SKYBOX
//#define ROOM
//#define BEAR
//#define BEAR_X_RAY
//#define POST_EFFECTS
#define MAN
#define PARTICLE_SYSTEM
#define FONT
//#define BUMPED
//#define DEFERRED_RENDERING

static Game* gameInstance;

Game* Game::get() {
	return gameInstance;
}

Game::Game(QWidget* parent) : QDockWidget(parent), initialized_(false) {
	gameInstance = this;
	grayscale_ = new Grayscale;
	inversion_ = new Inversion;
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
}

void Game::awake() {
	createScene();
	update();
}

void Game::start() {
}

void Game::update() {
	ui_->canvas->redraw();
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

uint roomEntityID;

void Game::createScene() {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(0.15f));
	DirectionalLight light = NewDirectionalLight();
	light->SetColor(glm::vec3(0.7f));

	Camera camera = NewCamera();
	controller_->setCamera(camera);

	light->SetParent(camera);

#ifdef DEFERRED_RENDERING
	camera->SetRenderPath(RenderPathDeferred);
#endif

	camera->SetPosition(glm::vec3(0, 25, 0));

#ifdef POST_EFFECTS
	//camera->AddImageEffect(inversion_);
	camera->AddImageEffect(grayscale_);
#endif

#ifdef SKYBOX
	camera->SetClearType(ClearTypeSkybox);
	Skybox skybox = NewSkybox();
	std::string faces[] = {
		"textures/lake_skybox/right.jpg",
		"textures/lake_skybox/left.jpg",
		"textures/lake_skybox/top.jpg",
		"textures/lake_skybox/bottom.jpg",
		"textures/lake_skybox/back.jpg",
		"textures/lake_skybox/front.jpg",
	};

	skybox->Load(faces);

	camera->SetSkybox(skybox);
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
	particleSystem->SetPosition(glm::vec3(-30, 20, -50));

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
	fentity->SetPosition(glm::vec3(-10, 20, -20));
	fentity->SetEulerAngles(glm::vec3(0, 0, 0));

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
	Material fontMaterial = dsp_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial->SetColor4(Variables::mainColor, glm::vec4(1, 0, 0, 1));

	/*
	Renderer renderer2 = NewMeshRenderer();
	Material fontMaterial2 = dsp_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial2->SetColor(Variables::mainColor, glm::vec4(1, 1, 0, 1));
	*/

	renderer->AddMaterial(fontMaterial);
	//renderer2->AddMaterial(fontMaterial2);

	fentity->SetRenderer(renderer);
	//fentity2->SetRenderer(renderer2);
#endif

#ifdef ROOM
 	Entity room = WorldInstance()->Import("models/room_thickwalls.obj");
 	room->SetPosition(glm::vec3(0, 25, -65));
 	room->SetEulerAngles(glm::vec3(30, 60, 0));
 	roomEntityID = room->GetInstanceID();
#endif

#ifdef BEAR
	Entity bear = WorldInstance()->Import("models/teddy_bear.fbx");
	bear->SetPosition(glm::vec3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Resources::FindShader("shaders/xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef MAN
	Entity man = WorldInstance()->Import("models/boblampclean.md5mesh");
	man->SetPosition(glm::vec3(0, 0, -70));
	man->SetEulerAngles(glm::vec3(270, 180, 180));
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
