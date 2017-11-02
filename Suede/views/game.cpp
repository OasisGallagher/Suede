#include <QWidget>

#include "game.h"
#include "light.h"
#include "skybox.h"
#include "camera.h"
#include "shader.h"
#include "skybox.h"
#include "engine.h"
#include "texture.h"
#include "surface.h"
#include "particlesystem.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/cameracontroller.h"

Game* Game::get() {
	static Game instance;
	return &instance;
}

Game::Game() : sceneCreated_(false) {
	grayscale_ = new Grayscale;
	inversion_ = new Inversion;
	controller_ = new CameraController;
}

Game::~Game() {
	delete grayscale_;
	delete inversion_;
	delete controller_;
}

void Game::initialize() {
	canvas_ = view_->findChild<QWidget*>("canvas", Qt::FindDirectChildrenOnly);
	canvas_->installEventFilter(this);
	connect(canvas_, SIGNAL(repaint()), this, SLOT(update()), Qt::DirectConnection);
	view_->setSize(300, 400);
}

bool Game::eventFilter(QObject* watched, QEvent* event) {
	if (watched != canvas_) {
		return false;
	}

	switch (event->type()) {
	case QEvent::Wheel:
		wheelEvent((QWheelEvent*)event);
		break;
	case QEvent::MouseButtonPress:
		mousePressEvent((QMouseEvent*)event);
		break;
	case QEvent::MouseButtonRelease:
		mouseReleaseEvent((QMouseEvent*)event);
		break;
	case QEvent::MouseMove:
		mouseMoveEvent((QMouseEvent*)event);
		break;
	case QEvent::KeyRelease:
		keyPressEvent((QKeyEvent*)event);
		break;
	}

	return false;
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

void Game::update() {
	if (!sceneCreated_) {
		createScene();
		sceneCreated_ = true;
	}
}

void Game::createScene() {
	World world = Engine::get()->world();

	world->GetEnvironment()->SetAmbientColor(glm::vec3(0.15f));
	DirectionalLight light = dsp_cast<DirectionalLight>(world->Create(ObjectTypeDirectionalLight));
	light->SetColor(glm::vec3(0.7f));

	Camera camera = dsp_cast<Camera>(world->Create(ObjectTypeCamera));
	controller_->setCamera(camera);

	//camera->AddPostEffect(inversion_);
	//camera->AddPostEffect(grayscale_);
	//camera->SetPosition(glm::vec3(0, 1, 5));

	//glm::quat q(glm::lookAt(glm::vec3(0, 1, 5), glm::vec3(0), glm::vec3(0, 1, 0)));
	//camera->SetRotation(q);

	camera->SetPosition(glm::vec3(0, 25, 0));

	camera->SetClearType(ClearTypeSkybox);
	//camera->SetClearColor(glm::vec3(0, 0, 0.4f));

	Skybox skybox = dsp_cast<Skybox>(world->Create(ObjectTypeSkybox));
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

	RenderTexture renderTexture = dsp_cast<RenderTexture>(world->Create(ObjectTypeRenderTexture));
	renderTexture->Load(RenderTextureFormatRgba, canvas_->width(), canvas_->height());
	//camera->SetRenderTexture(renderTexture);
	//camera->SetClearColor(glm::vec3(0.0f, 0.0f, 0.4f));

	ParticleSystem particleSystem = dsp_cast<ParticleSystem>(world->Create(ObjectTypeParticleSystem));
	particleSystem->SetPosition(glm::vec3(0, 20, -50));

	SphereParticleEmitter emitter = dsp_cast<SphereParticleEmitter>(world->Create(ObjectTypeSphereParticleEmitter));
	emitter->SetRadius(5);
	emitter->SetRate(2000);
	emitter->SetStartColor(glm::vec4(1, 1, 1, 0.5f));
	emitter->SetStartDuration(2);
	emitter->SetStartSize(1);
	emitter->SetStartVelocity(glm::vec3(0));
	ParticleBurst burst = { 4, 3, 20 };
	particleSystem->SetEmitter(emitter);

	particleSystem->SetGravityScale(0);

	ParticleAnimator animator = dsp_cast<ParticleAnimator>(world->Create(ObjectTypeParticleAnimator));
	particleSystem->SetMaxParticles(1000);
	particleSystem->SetDuration(5);
	particleSystem->SetLooping(true);

	//Sprite sprite = dsp_cast<Sprite>(world->Create(ObjectTypeSprite));
	//sprite->SetParent(camera);
	//light->SetParent(camera);
	//sprite->SetPosition(glm::vec3(0, 0, -70));
	//sprite->SetEulerAngles(glm::vec3(270, 180, 180));

	/* Mesh.
	Mesh mesh = dynamic_ptr_cast<Mesh>(world->Create("Mesh"));
	SurfaceAttribute attribute;
	attribute.positions.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
	attribute.positions.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
	attribute.positions.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	attribute.indexes.push_back(0);
	attribute.indexes.push_back(1);
	attribute.indexes.push_back(2);

	surface->SetAttribute(attribute);
	mesh->SetTriangles(3, 0, 0);
	surface->AddMesh(mesh);
	*/
	/*
	sprite->LoadModel("models/boblampclean.md5mesh");
	sprite->GetAnimation()->SetWrapMode(AnimationWrapModePingPong);
	*/
	//sprite->GetAnimation()->Play("");
	//Surface surface = sprite->GetSurface();

	//Texture2D albedo = dsp_cast<Texture2D>(world->Create(ObjectTypeTexture2D));
	//albedo->Load("textures/room_uvmap.dds");

	//Texture2D bump = dsp_cast<Texture2D>(world->Create(ObjectTypeTexture2D));
	//bump->Load("textures/bump.bmp");

	//MaterialTextures& textures = surface->GetMesh(0)->GetMaterialTextures();
	//textures.albedo = albedo;
	//textures.bump = bump;

	/*Renderer renderer = sprite->GetRenderer();
	renderer->SetRenderState(Cull, Off);
	renderer->SetRenderState(DepthTest, LessEqual);

	Shader shader = dsp_cast<Shader>(world->Create(ObjectTypeShader));
	shader->Load("buildin/shaders/lit_texture");
	renderer->GetMaterial(0)->SetShader(shader);*/
}