#include <QWidget>
#include "canvas.h"

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

//#define SKYBOX
#define ROOM
//#define BEAR
//#define BEAR_X_RAY
//#define POST_EFFECTS
//#define MAN
//#define PARTICLE_SYSTEM
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
void Game::init() {
	canvas_ = findChild<Canvas*>("canvas", Qt::FindChildrenRecursively);
	updateTimer_ = startTimer(10, Qt::PreciseTimer);
}

void Game::awake() {
	TagManager::Register(Tags::kHideInHierarchy);

	createScene();
	update();
}

void Game::start() {
}

void Game::update() {
	canvas_->redraw();
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

uint ballSpriteID;
uint roomSpriteID;

void Game::createScene() {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(0.15f));
	DirectionalLight light = NewDirectionalLight();
	light->SetColor(glm::vec3(0.7f));

	Camera camera = NewCamera();
	camera->SetTag(Tags::kHideInHierarchy);

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
	skybox->SetTag(Tags::kHideInHierarchy);

	camera->SetSkybox(skybox);
#else
	camera->SetClearType(ClearTypeColor);
	camera->SetClearColor(glm::vec3(0, 0, 0.1f));
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = NewRenderTexture();
	renderTexture->Load(RenderTextureFormatRgba, canvas_->width(), canvas_->height());
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
	Sprite fsprite = NewSprite();
	fsprite->SetPosition(glm::vec3(-10, 20, -20));
	fsprite->SetEulerAngles(glm::vec3(0, 0, 0));

	Sprite fsprite2 = NewSprite();
	fsprite2->SetPosition(glm::vec3(-10, 30, -20));

	fsprite->SetTag(Tags::kHideInHierarchy);
	fsprite2->SetTag(Tags::kHideInHierarchy);

	Font font = NewFont();
	font->Load("fonts/ms_yh.ttf", 12);

	TextMesh mesh = NewTextMesh();
	mesh->SetFont(font);
	mesh->SetText("��ϼ�������� ��ˮ������һɫ");
	
	mesh->SetFontSize(12);
	fsprite->SetMesh(mesh);
	fsprite2->SetMesh(mesh);

	Renderer renderer = NewMeshRenderer();
	Material fontMaterial = dsp_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial->SetVector4(Variables::mainColor, glm::vec4(1, 0, 0, 1));

	Renderer renderer2 = NewMeshRenderer();
	Material fontMaterial2 = dsp_cast<Material>(font->GetMaterial()->Clone());
	fontMaterial2->SetVector4(Variables::mainColor, glm::vec4(1, 1, 0, 1));

	renderer->AddMaterial(fontMaterial);
	renderer2->AddMaterial(fontMaterial2);

	fsprite->SetRenderer(renderer);
	fsprite2->SetRenderer(renderer2);
#endif

#ifdef ROOM
 	Sprite room = WorldInstance()->Import("models/room_thickwalls.obj");
 	room->SetPosition(glm::vec3(0, 25, -65));
 	room->SetEulerAngles(glm::vec3(30, 60, 0));
 	roomSpriteID = room->GetInstanceID();

	Sprite ball = WorldInstance()->Import("models/box.obj");
	ball->SetPosition(glm::vec3(0, 25, -55));
	ball->SetScale(glm::vec3(0.5f, 2.5f, 0.5f));
	ball->SetEulerAngles(glm::vec3(90, 0, 0));
	ballSpriteID = ball->GetInstanceID();
#endif

#ifdef BEAR
	Sprite bear = WorldInstance()->Import("models/teddy_bear.fbx");
	bear->SetPosition(glm::vec3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Resources::FindShader("shaders/xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef MAN
	Sprite man = WorldInstance()->Import("models/boblampclean.md5mesh");
	man->SetPosition(glm::vec3(0, 0, -70));
	man->SetEulerAngles(glm::vec3(270, 180, 180));
	if (man) {
		//sprite->SetParent(camera);

		Animation animation = man->GetAnimation();
		if (animation) {
			animation->SetWrapMode(AnimationWrapModePingPong);
			animation->Play("");
		}
	}
#endif
}
