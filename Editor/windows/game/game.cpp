#include <QWidget>
#include <QToolBar>
#include <glm/gtc/matrix_transform.hpp>

#include "canvas.h"
#include "ui_editor.h"
#include "widgets/status/statusbar.h"
#include "widgets/statistics/statswidget.h"

#include "game.h"
#include "font.h"
#include "mesh.h"
#include "light.h"
#include "input.h"
#include "time2.h"
#include "world.h"
#include "input.h"
#include "screen.h"
#include "camera.h"
#include "shader.h"
#include "engine.h"
#include "gizmos.h"
#include "texture.h"
#include "physics.h"
#include "resources.h"
#include "projector.h"
#include "behaviour.h"
#include "statistics.h"
#include "tagmanager.h"
#include "particlesystem.h"
#include "builtinproperties.h"

#include "tools/math2.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/gaussianblur.h"
#include "scripts/selectiongizmos.h"
#include "scripts/cameracontroller.h"

//#define ROOM
//#define SKYBOX
//#define PROJECTOR
//#define PROJECTOR_ORTHOGRAPHIC
//#define BEAR
//#define BEAR_X_RAY
//#define IMAGE_EFFECTS
//#define ANIMATION
//#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED
//#define NORMAL_VISUALIZER
//#define DEFERRED_RENDERING

static const char* roomFbxPath = "room.fbx";
static const char* bumpedFbxPath = "builtin/sphere.fbx";
static const char* normalVisualizerFbxPath = "nanosuit.fbx";

static const char* manFbxPath = "boblampclean.md5mesh";
static const char* lightModelPath = "builtin/sphere.fbx";

Game::Game(QWidget* parent) : QDockWidget(parent), canvas_(nullptr), stat_(nullptr), controller_(nullptr) {
}

Game::~Game() {
}

Canvas* Game::canvas() {
	if (canvas_ == nullptr) {
		canvas_ = findChild<Canvas*>("canvas");
	}

	return canvas_;
}

void Game::init(Ui::Editor* ui) {
	WinBase::init(ui);
	stat_ = new StatsWidget(this);
	stat_->setVisible(false);

	connect(ui_->stat, SIGNAL(stateChanged(int)), this, SLOT(onToggleStat(int)));
	connect(Hierarchy::instance(), SIGNAL(focusGameObject(GameObject)), this, SLOT(onFocusGameObjectBounds(GameObject)));
	connect(Hierarchy::instance(), SIGNAL(selectionChanged(const QList<GameObject>&, const QList<GameObject>&)),
		this, SLOT(onSelectionChanged(const QList<GameObject>&, const QList<GameObject>&)));

	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(updateStatContent()));
	timer_->start(800);

	connect(ui_->shadingMode, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onShadingModeChanged(const QString&)));
}

template <class T>
Object CreateComponent() { return std::make_shared<T>(); }
#define REGISTER_COMPONENT(T)	ComponentUtility::Register(#T, CreateComponent<T>)

void Game::awake() {
	REGISTER_COMPONENT(CameraController);

	ui_->shadingMode->setEnums(+Graphics::GetShadingMode());
	createScene();

	input_ = new QtInputDelegate(ui_->canvas);
	Input::SetDelegate(input_);
}

void Game::tick() {
	if (Input::GetMouseButtonUp(0)) {
		RaycastHit hitInfo;
		glm::vec3 src = CameraUtility::GetMain()->GetTransform()->GetPosition();
		glm::vec3 dest = CameraUtility::GetMain()->ScreenToWorldPoint(glm::vec3(Input::GetMousePosition(), 1));

		if (Physics::Raycast(Ray(src, dest - src), 1000, &hitInfo)) {
			Hierarchy::instance()->setSelectedGameObjects(QList<GameObject>{ hitInfo.gameObject });
		}
	}
}

void Game::OnGameObjectImported(GameObject root, const std::string& path) {
	//root->GetTransform()->SetParent(World::GetRootTransform());
	root->SetName(path);

	if (path == manFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 0, -70));
		root->GetTransform()->SetEulerAngles(glm::vec3(270, 180, 180));
		root->GetTransform()->SetScale(glm::vec3(0.2f));
		//go->SetParent(camera);

		Animation animation = root->GetComponent<Animation>();
		if (animation) {
			animation->SetWrapMode(AnimationWrapMode::PingPong);
			animation->Play("");
		}
	}
	else if (path == roomFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 25, -65));
		root->GetTransform()->SetEulerAngles(glm::vec3(30, 0, 0));
		if (path.find("house") != std::string::npos) {
			root->GetTransform()->SetScale(glm::vec3(0.01f));
		}
		else if (path.find("suzanne") != std::string::npos) {
			Texture2D diffuse = NewTexture2D();
			diffuse->Load("suzanne/diffuse.dds");
			GameObject target = root->GetTransform()->FindChild("suzanne_root/default")->GetGameObject();

			Material material = target->GetComponent<MeshRenderer>()->GetMaterial(0);
			material->SetTexture(BuiltinProperties::MainTexture, diffuse);

			root->GetTransform()->SetPosition(glm::vec3(0, 25, -5));
			root->GetTransform()->SetEulerAngles(glm::vec3(0));
		}
	}
	else if (path == bumpedFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 25, -15));

		GameObject target = root->GetTransform()->FindChild("Sphere01")->GetGameObject();
		Material material = target->GetComponent<MeshRenderer>()->GetMaterial(0);
		material->SetShader(Resources::FindShader("builtin/lit_bumped_texture"));

		Texture2D diffuse = NewTexture2D();
		diffuse->Load("bumped/diffuse.jpg");
		material->SetTexture(BuiltinProperties::MainTexture, diffuse);

		Texture2D normal = NewTexture2D();
		normal->Load("bumped/normal.jpg");
		material->SetTexture(BuiltinProperties::BumpTexture, normal);
	}
	else if (path == normalVisualizerFbxPath) {
		root->GetTransform()->SetPosition(glm::vec3(0, 25, -5));
		root->GetTransform()->SetEulerAngles(glm::vec3(0));

		GameObject target = root->GetTransform()->FindChild("nanosuit_root/default")->GetGameObject();

		for (Material material : target->GetComponent<MeshRenderer>()->GetMaterials()) {
			//material->SetShader(Resources::FindShader("builtin/normal_visualizer"));
		}
	}
}

// void Game::OnFrameLeave() {
// //	Graphics::Blit(targetTexture_, nullptr);
// }

void Game::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Space:
			break;
	}
}

void Game::resizeEvent(QResizeEvent* event) {
	updateStatPosition();

	// SUEDE TODO: Canvas::resizeGL not called when game window resized.
	const QSize& size = event->size();
	canvas()->sizeChanged(size.width(), size.height());
}

void Game::timerEvent(QTimerEvent *event) {
}

void Game::onToggleStat(int state) {
	stat_->setVisible(!!state);

	if (stat_->isVisible()) {
		updateStatContent();
		updateStatPosition();
	}
}

void Game::updateStatPosition() {
	QPoint pos = ui_->stat->parentWidget()->mapTo(this, ui_->stat->pos());
	pos.setX(pos.x() - stat_->width());
	pos.setY(pos.y() + ui_->stat->height());
	stat_->move(pos);
}

void Game::onShadingModeChanged(const QString& str) {
	Graphics::SetShadingMode(ShadingMode::from_string(str.toLatin1()));
}

void Game::onFocusGameObjectBounds(GameObject go) {
	glm::vec3 center = go->GetBounds().center;
	Transform camera = CameraUtility::GetMain()->GetTransform();

	float distance = calculateCameraDistanceFitsBounds(CameraUtility::GetMain(), go->GetBounds());
	camera->SetPosition(center + go->GetTransform()->GetForward() * distance);

	glm::quat q(glm::lookAt(camera->GetPosition(), center, glm::vec3(0, 1, 0)));
	camera->SetRotation(glm::conjugate(q));
}

void Game::onSelectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected) {
	gizmos_->setSelection(Hierarchy::instance()->selectedGameObjects());
}

float Game::calculateCameraDistanceFitsBounds(Camera camera, const Bounds& bounds) {
	float f = tanf(camera->GetFieldOfView() / 2.f);
	float dy = 2 * bounds.size.y / f;
	float dx = 2 * bounds.size.x / (f * camera->GetAspect());
	return Math::Clamp(qMax(dx, dy), camera->GetNearClipPlane() + bounds.size.z * 2, camera->GetFarClipPlane() - bounds.size.z * 2);
}

void Game::updateStatContent() {
 	if (stat_->isVisible()) {
		stat_->updateContent();
 	}
}

void Game::createScene() {
	GameObject lightGameObject = NewGameObject();
	lightGameObject->SetName("light");

	Light light = lightGameObject->AddComponent<Light>();
	light->SetColor(Color(0.7f, 0.7f, 0.7f, 1));
	light->GetTransform()->SetParent(World::GetRootTransform());

	/*World::ImportTo(light, lightModelPath, this);*/

	GameObject cameraGameObject = NewGameObject();
	cameraGameObject->SetName("camera");

	Camera camera = cameraGameObject->AddComponent<Camera>();
	CameraUtility::SetMain(camera);
	camera->GetTransform()->SetParent(World::GetRootTransform());

	/*RenderTexture targetTexture = NewRenderTexture();
	targetTexture->Create(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight());
	camera->SetTargetTexture(targetTexture);*/

	controller_ = cameraGameObject->AddComponent<CameraController>().get();
	controller_->setView(this);

	gizmos_ = cameraGameObject->AddComponent<SelectionGizmos>().get();

#ifdef PROJECTOR
	Projector projector = NewProjector();
	
#ifdef PROJECTOR_ORTHOGRAPHIC
	projector->SetPerspective(false);
	projector->SetOrthographicSize(5);
#else
	projector->SetFieldOfView(Math::Radians(9.f));
#endif
	projector->GetTransform()->SetParent(World::GetRootTransform());
	projector->GetTransform()->SetPosition(glm::vec3(0, 25, 0));

	Texture2D texture = NewTexture2D();
	texture->Load("brick_diffuse.jpg");
	projector->SetTexture(texture);
#endif // PROJECTOR

//	light->GetTransform()->SetParent(camera->GetTransform());

#ifdef DEFERRED_RENDERING
	camera->SetRenderPath(RenderPathDeferred);
#endif

	camera->SetFarClipPlane(10000.f);
	camera->GetTransform()->SetPosition(glm::vec3(0, 25, 0));
	//camera->SetDepthTextureMode(DepthTextureMode::Depth);

	Graphics::SetAmbientOcclusionEnabled(true);

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
	cameraGameObject->AddComponent<GaussianBlur>();
	//camera->AddComponent<Grayscale>();
	//camera->AddComponent<Inversion>();
#endif

	camera->SetClearColor(Color(0, 0.1f, 0.1f, 1));

	Material skybox = NewMaterial();
	skybox->SetShader(Resources::FindShader("builtin/skybox"));

	TextureCube cube = NewTextureCube();

	std::string faces[] = {
		"lake_skybox/right.jpg",
		"lake_skybox/left.jpg",
		"lake_skybox/top.jpg",
		"lake_skybox/bottom.jpg",
		"lake_skybox/back.jpg",
		"lake_skybox/front.jpg",
	};

	cube->Load(faces);
	skybox->SetTexture(BuiltinProperties::MainTexture, cube);
	skybox->SetColor(BuiltinProperties::MainColor, Color::white);
	Environment::SetSkybox(skybox);

#ifdef SKYBOX
	camera->SetClearType(ClearType::Skybox);
#else
	camera->SetClearType(ClearType::Color);
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = NewRenderTexture();
	renderTexture->Load(RenderTextureFormatRgba, ui_->canvas->width(), ui_->canvas->height());
	camera->SetRenderTexture(renderTexture);
#endif
	
#ifdef PARTICLE_SYSTEM
	GameObject go = NewGameObject();
	ParticleSystem particleSystem = go->AddComponent<IParticleSystem>();
	go->GetTransform()->SetPosition(glm::vec3(-30, 20, -50));
	go->GetTransform()->SetParent(World::GetRootTransform());

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

	GameObject redText = NewGameObject();
	redText->SetName("RedText");
	redText->GetTransform()->SetPosition(glm::vec3(-10, 20, -20));
	redText->GetTransform()->SetParent(World::GetRootTransform());

	GameObject blueText = NewGameObject();
	blueText->SetName("BlueText");
	blueText->GetTransform()->SetPosition(glm::vec3(-10, 30, -20));
	blueText->GetTransform()->SetParent(World::GetRootTransform());

	TextMesh redMesh = redText->AddComponent<ITextMesh>();
	redMesh->SetFont(font);
	redMesh->SetText("落霞与孤鹜齐飞");
	redMesh->SetFontSize(12);

	TextMesh blueMesh = blueText->AddComponent<ITextMesh>();
	blueMesh->SetFont(font);
	blueMesh->SetText("秋水共长天一色");
	blueMesh->SetFontSize(12);

	Renderer redRenderer = redText->AddComponent<IMeshRenderer>();
	Material redMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	redMaterial->SetColor(BuiltinProperties::MainColor, Color(1, 0, 0, 1));
	redRenderer->AddMaterial(redMaterial);

	Renderer blueRenderer = blueText->AddComponent<IMeshRenderer>();
	Material blueMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	blueMaterial->SetColor(BuiltinProperties::MainColor, Color(0, 0, 1, 1));
	blueRenderer->AddMaterial(blueMaterial);

#endif

#ifdef ROOM
	GameObject room = World::Import(roomFbxPath, this);
#endif

#ifdef BUMPED
	GameObject bumped = World::Import(bumpedFbxPath, this);
#endif

#ifdef NORMAL_VISUALIZER
	GameObject normalVisualizer = World::Import(normalVisualizerFbxPath, this);
#endif

#ifdef BEAR
	GameObject bear = World::Import("teddy_bear.fbx");
	bear->GetTransform()->SetPosition(glm::vec3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Resources::FindShader("xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef ANIMATION
	GameObject man = World::Import(manFbxPath, this);
#endif
}
