#include <QWidget>

#include "game.h"
#include "font.h"
#include "mesh.h"
#include "light.h"
#include "skybox.h"
#include "camera.h"
#include "shader.h"
#include "skybox.h"
#include "engine.h"
#include "texture.h"
#include "variables.h"
#include "particlesystem.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/cameracontroller.h"

//#define SKYBOX
#define MODEL
//#define POST_EFFECTS
//#define ANIMATION
//#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED

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

	case QEvent::Resize:
		resizeEvent((QResizeEvent*)event);
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

void Game::resizeEvent(QResizeEvent* event) {
	controller_->onResize(event->size());
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
	camera->SetPosition(glm::vec3(0, 25, 0));

#ifdef POST_EFFECTS
	camera->AddPostEffect(inversion_);
	camera->AddPostEffect(grayscale_);
#endif

#ifdef SKYBOX
	camera->SetClearType(ClearTypeSkybox);
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
#else
	camera->SetClearType(ClearTypeColor);
	camera->SetClearColor(glm::vec3(0, 0, 0.1f));
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = dsp_cast<RenderTexture>(world->Create(ObjectTypeRenderTexture));
	renderTexture->Load(RenderTextureFormatRgba, canvas_->width(), canvas_->height());
	camera->SetRenderTexture(renderTexture);
#endif

#ifdef PARTICLE_SYSTEM
	ParticleSystem particleSystem = dsp_cast<ParticleSystem>(world->Create(ObjectTypeParticleSystem));
	particleSystem->SetPosition(glm::vec3(-30, 20, -50));

	SphereParticleEmitter emitter = dsp_cast<SphereParticleEmitter>(world->Create(ObjectTypeSphereParticleEmitter));
	emitter->SetRadius(5);
	emitter->SetRate(200);
	emitter->SetStartColor(glm::vec4(1, 1, 1, 0.5f));
	emitter->SetStartDuration(2);
	emitter->SetStartSize(1);
	emitter->SetStartVelocity(glm::vec3(0, 1, 0));
	ParticleBurst burst = { 4, 3, 20 };
	particleSystem->SetEmitter(emitter);

	ParticleAnimator animator = dsp_cast<ParticleAnimator>(world->Create(ObjectTypeParticleAnimator));
	animator->SetGravityScale(0.2f);
	particleSystem->SetParticleAnimator(animator);

	particleSystem->SetMaxParticles(1000);
	particleSystem->SetDuration(5);
	particleSystem->SetLooping(true);
#endif

#if defined(FONT)
	Sprite fsprite = dsp_cast<Sprite>(world->Create(ObjectTypeSprite));
	fsprite->SetPosition(glm::vec3(0, 20, -20));
	fsprite->SetEulerAngles(glm::vec3(0, 0, 0));

	Font font = dsp_cast<Font>(world->Create(ObjectTypeFont));
	font->Load("fonts/ms_yh.ttf", 12);

	TextMesh mesh = dsp_cast<TextMesh>(world->Create(ObjectTypeTextMesh));
	mesh->SetFont(font);
	mesh->SetText("落霞与孤鹜齐飞 秋水共长天一色");
	
	std::vector<uchar> data;
	dsp_cast<Texture2D>(mesh->GetFont()->GetMaterial()->GetTexture("c_mainTexture"))->EncodeToJpg(data);
	QImage image;
	if (image.loadFromData(&data[0], data.size())) {
		image.save("C:\\Users\\Gallagher\\Desktop\\1.jpg");
	}
	
	mesh->SetFontSize(12);
	fsprite->SetMesh(mesh);

	Renderer renderer = dsp_cast<MeshRenderer>(world->Create(ObjectTypeMeshRenderer));
	renderer->AddMaterial(font->GetMaterial());
	fsprite->SetRenderer(renderer);
#endif

#if defined(MODEL) || defined(ANIMATION)
	Sprite sprite;
#if defined(MODEL)
	sprite = world->Import("models/jeep.fbx");
	sprite->SetPosition(glm::vec3(0, 0, -120));
	sprite->SetEulerAngles(glm::vec3(0, 180, 0));
#elif defined(ANIMATION)
	sprite = world->Import("models/boblampclean.md5mesh");
	sprite->SetPosition(glm::vec3(0, 0, -70));
	sprite->SetEulerAngles(glm::vec3(270, 180, 180));
#endif

	if (sprite) {
		//sprite->SetParent(camera);
		//light->SetParent(camera);

		Animation animation = sprite->GetAnimation();
		if (animation) {
			animation->SetWrapMode(AnimationWrapModePingPong);
			animation->Play("");
		}
	}

#endif
}
