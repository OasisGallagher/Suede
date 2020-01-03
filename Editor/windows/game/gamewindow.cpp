#include "gamewindow.h"

#include <QWidget>
#include <QToolBar>

#include "canvas.h"
#include "ui_editor.h"
#include "main/editor.h"
#include "widgets/status/statusbar.h"
#include "widgets/statistics/statswidget.h"

#include "tags.h"
#include "font.h"
#include "mesh.h"
#include "scene.h"
#include "light.h"
#include "input.h"
#include "time2.h"
#include "engine.h"
#include "input.h"
#include "screen.h"
#include "camera.h"
#include "shader.h"
#include "gizmos.h"
#include "texture.h"
#include "physics.h"
#include "resources.h"
#include "projector.h"
#include "behaviour.h"
#include "particlesystem.h"
#include "builtinproperties.h"

#include "math/mathf.h"

#include "scripts/grayscale.h"
#include "scripts/inversion.h"
#include "scripts/gaussianblur.h"
#include "scripts/selectiongizmos.h"
#include "scripts/cameracontroller.h"

#define ROOM
//#define SKYBOX
//#define PROJECTOR
//#define PROJECTOR_ORTHOGRAPHIC
//#define BEAR
//#define BEAR_X_RAY
//#define IMAGE_EFFECTS
#define ANIMATION
#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED
//#define NORMAL_VISUALIZER
//#define DEFERRED_RENDERING

static const char* roomFbxPath = "room.fbx";
static const char* bumpedFbxPath = "builtin/sphere.fbx";
static const char* normalVisualizerFbxPath = "nanosuit.fbx";

static const char* manFbxPath = "boblampclean.md5mesh";
static const char* lightModelPath = "builtin/sphere.fbx";

GameWindow::GameWindow(QWidget* parent)
	: ChildWindow(parent)
	, canvas_(nullptr), stat_(nullptr), controller_(nullptr), inputDelegate_(nullptr) {
	stat_ = new StatsWidget(this);
	stat_->setVisible(false);
}

GameWindow::~GameWindow() {
	delete inputDelegate_;
}

void GameWindow::initUI() {
	canvas_ = findChild<Canvas*>("canvas");

	connect(ui_->stat, SIGNAL(stateChanged(int)), this, SLOT(onToggleStat(int)));

	HierarchyWindow* hw = editor_->childWindow<HierarchyWindow>();
	connect(hw, SIGNAL(focusGameObject(GameObject*)), this, SLOT(onFocusGameObjectBounds(GameObject*)));
	connect(hw, SIGNAL(selectionChanged(const QList<GameObject*>&, const QList<GameObject*>&)),
		this, SLOT(onSelectionChanged(const QList<GameObject*>&, const QList<GameObject*>&)));

	connect(ui_->shadingMode, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onShadingModeChanged(const QString&)));
}

void GameWindow::awake() {
	Component::Register<CameraController>();

	ui_->shadingMode->setEnums(+Engine::GetSubsystem<Graphics>()->GetShadingMode());

	input_ = Engine::GetSubsystem<Input>();

	inputDelegate_ = new QtInputDelegate(ui_->canvas);
	input_->SetDelegate(inputDelegate_);

	setupScene();
}

void GameWindow::tick() {
	HierarchyWindow* hw = editor_->childWindow<HierarchyWindow>();
	controller_->setSelection(hw->selectedGameObject());
	gizmos_->setSelection(hw->selectedGameObjects());

	if (input_->GetMouseButtonUp(0)) {
		RaycastHit hitInfo;
		Vector3 src = Camera::GetMain()->GetTransform()->GetPosition();
		Vector2 mousePosition = input_->GetMousePosition();
		Vector3 dest = Camera::GetMain()->ScreenToWorldPoint(Vector3(mousePosition.x, mousePosition.y, 1));

		if (Engine::GetSubsystem<Physics>()->Raycast(Ray(src, dest - src), 1000, &hitInfo)) {
			editor_->childWindow<HierarchyWindow>()->setSelectedGameObjects(QList<GameObject*>{ hitInfo.gameObject });
		}
	}
}

void GameWindow::onGameObjectImported(GameObject* root, const std::string& path) {
	Debug::Log("\"%s\" loaded", path.c_str());

	root->SetName(path);

	if (path == manFbxPath) {
		root->GetTransform()->SetPosition(Vector3(0, 0, -70));
		root->GetTransform()->SetEulerAngles(Vector3(270, 180, 180));
		root->GetTransform()->SetScale(Vector3(0.2f));
		//go->SetParent(camera);

		Animation* animation = root->GetComponent<Animation>();
		if (animation) {
			animation->SetWrapMode(AnimationWrapMode::PingPong);
			animation->Play("");
		}
	}
	else if (path == roomFbxPath) {
		root->GetTransform()->SetPosition(Vector3(0, 25, -65));
		root->GetTransform()->SetEulerAngles(Vector3(30, 0, 0));
		if (path.find("house") != std::string::npos) {
			root->GetTransform()->SetScale(Vector3(0.01f));
		}
		else if (path.find("suzanne") != std::string::npos) {
			ref_ptr<Texture2D> diffuse = new Texture2D();
			diffuse->Load("suzanne/diffuse.dds");
			GameObject* target = root->GetTransform()->FindChild("suzanne_root/default")->GetGameObject();

			Material* material = target->GetComponent<MeshRenderer>()->GetMaterial(0);
			material->SetTexture(BuiltinProperties::MainTexture, diffuse.get());

			root->GetTransform()->SetPosition(Vector3(0, 25, -5));
			root->GetTransform()->SetEulerAngles(Vector3(0));
		}
	}
	else if (path == bumpedFbxPath) {
		root->GetTransform()->SetPosition(Vector3(0, 25, -15));

		GameObject* target = root->GetTransform()->FindChild("Sphere01")->GetGameObject();
		Material* material = target->GetComponent<MeshRenderer>()->GetMaterial(0);
		material->SetShader(Shader::Find("builtin/lit_bumped_texture"));

		ref_ptr<Texture2D> diffuse = new Texture2D();
		diffuse->Load("bumped/diffuse.jpg");
		material->SetTexture(BuiltinProperties::MainTexture, diffuse.get());

		ref_ptr<Texture2D> normal = new Texture2D();
		normal->Load("bumped/normal.jpg");
		material->SetTexture(BuiltinProperties::BumpTexture, normal.get());
	}
	else if (path == normalVisualizerFbxPath) {
		root->GetTransform()->SetPosition(Vector3(0, 25, -5));
		root->GetTransform()->SetEulerAngles(Vector3(0));

		GameObject* target = root->GetTransform()->FindChild("nanosuit_root/default")->GetGameObject();

		MeshRenderer* renderer = target->GetComponent<MeshRenderer>();
		for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
			Material* material = renderer->GetMaterial(i);
			material->SetShader(Shader::Find("builtin/normal_visualizer"));
		}
	}
}

// void Game::OnFrameLeave() {
// //	Graphics::Blit(targetTexture_, nullptr);
// }

void GameWindow::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Space:
			break;
	}
}

void GameWindow::resizeEvent(QResizeEvent* event) {
	updateStatPosition();
}

void GameWindow::timerEvent(QTimerEvent *event) {
}

void GameWindow::onToggleStat(int state) {
	stat_->setVisible(!!state);

	if (stat_->isVisible()) {
		updateStatPosition();
	}
}

void GameWindow::updateStatPosition() {
	QPoint pos = ui_->stat->parentWidget()->mapTo(this, ui_->stat->pos());
	pos.setX(pos.x() - stat_->width());
	pos.setY(pos.y() + ui_->stat->height());
	stat_->move(pos);
}

void GameWindow::onShadingModeChanged(const QString& str) {
	Engine::GetSubsystem<Graphics>()->SetShadingMode(ShadingMode::from_string(str.toLatin1()));
}

void GameWindow::onFocusGameObjectBounds(GameObject* go) {
	/*Vector3 center = go->GetBounds().center;
	Transform* camera = Camera::GetMain()->GetTransform();

	float distance = calculateCameraDistanceFitsBounds(Camera::GetMain(), go->GetBounds());
	camera->SetPosition(center + Vector3(0, 0, -1) * distance);

	Quaternion q(Matrix4::LookAt(camera->GetPosition(), center, Vector3(0, 1, 0)));
	camera->SetRotation(q.GetConjugated());*/
}

float GameWindow::calculateCameraDistanceFitsBounds(Camera* camera, const Bounds& bounds) {
	float f = tanf(camera->GetFieldOfView() / 2.f);
	float dy = 2 * bounds.size.y / f;
	float dx = 2 * bounds.size.x / (f * camera->GetAspect());
	return Mathf::Clamp(qMax(dx, dy), camera->GetNearClipPlane() + bounds.size.z * 2, camera->GetFarClipPlane() - bounds.size.z * 2);
}

void GameWindow::setupScene() {
	Debug::Log("test debug message");
	Debug::Log("test debug message2");

	Scene* scene = Engine::GetSubsystem<Scene>();

	ref_ptr<GameObject> lightGameObject = new GameObject();
	lightGameObject->SetName("light");

	Light* light = lightGameObject->AddComponent<Light>();
	light->SetColor(Color(0.7f, 0.7f, 0.7f, 1));
	light->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<GameObject> cameraGameObject = new GameObject();
	cameraGameObject->SetName("camera");

	Camera* camera = cameraGameObject->AddComponent<Camera>();
	Camera::SetMain(camera);
	camera->GetTransform()->SetParent(scene->GetRootTransform());

	controller_ = cameraGameObject->AddComponent<CameraController>();
	controller_->setView(this);

	gizmos_ = cameraGameObject->AddComponent<SelectionGizmos>();

#ifdef PROJECTOR
	ref_ptr<GameObject> projectorGameObject = new GameObject();
	Projector* projector = projectorGameObject->AddComponent<Projector>();
	
#ifdef PROJECTOR_ORTHOGRAPHIC
	projector->SetPerspective(false);
	projector->SetOrthographicSize(5);
#else
	projector->SetFieldOfView(Mathf::deg2Rad * 9.f);
#endif
	projector->GetTransform()->SetParent(scene->GetRootTransform());
	projector->GetTransform()->SetPosition(Vector3(0, 25, 0));

	ref_ptr<Texture2D> texture = new Texture2D();
	texture->Load("brick_diffuse.jpg");
	projector->SetTexture(texture.get());
#endif // PROJECTOR

//	light->GetTransform()->SetParent(camera->GetTransform());

#ifdef DEFERRED_RENDERING
	camera->SetRenderPath(RenderPathDeferred);
#endif

	camera->SetFarClipPlane(10000.f);
	camera->GetTransform()->SetPosition(Vector3(0, 25, 0));
	//camera->SetDepthTextureMode(DepthTextureMode::Depth);

	//Graphics::SetAmbientOcclusionEnabled(true);

	/*camera->SetRect(Rect(0.f, 0.f, 0.5f, 0.5f));*/
	//camera->SetActiveSelf(false);
	//camera->SetTargetTexture(targetTexture_);
	
	//Camera camera2 = NewCamera();
	//camera2->SetFarClipPlane(10000.f);
	//camera2->GetTransform()->SetPosition(Vector3(0, 25, 0));
	//camera2->SetDepthTextureMode(DepthTextureModeDepth);

	//camera2->SetRect(Rect(0.5f, 0.5f, 0.5f, 0.5f));
	////camera2->SetTargetTexture(targetTexture_);

	//camera2->SetClearColor(Vector3(0.1f, 0, 0.1f));
	//camera2->SetDepth(-1);
	//camera2->SetName("MultiCameraTest");

	light->GetTransform()->SetPosition(Vector3(0, 25, 0));

#ifdef IMAGE_EFFECTS
	cameraGameObject->AddComponent<GaussianBlur>();
	//camera->AddComponent<Grayscale>();
	//camera->AddComponent<Inversion>();
#endif

	camera->SetClearColor(Color(0, 0.3f, 0.5f, 1));

	ref_ptr<Material> skybox = new Material();
	skybox->SetShader(Shader::Find("builtin/skybox"));

	ref_ptr<TextureCube> cube = new TextureCube();

	std::string faces[] = {
		"lake_skybox/right.jpg",
		"lake_skybox/left.jpg",
		"lake_skybox/top.jpg",
		"lake_skybox/bottom.jpg",
		"lake_skybox/back.jpg",
		"lake_skybox/front.jpg",
	};

	cube->Load(faces);
	skybox->SetTexture(BuiltinProperties::MainTexture, cube.get());
	skybox->SetColor(BuiltinProperties::MainColor, Color::white);
	scene->GetEnvironment()->skybox = skybox;

#ifdef SKYBOX
	camera->SetClearType(ClearType::Skybox);
#else
	camera->SetClearType(ClearType::Color);
#endif
	
#ifdef RENDER_TEXTURE
	RenderTexture renderTexture = new RenderTexture();
	renderTexture->Load(RenderTextureFormatRgba, ui_->canvas->width(), ui_->canvas->height());
	camera->SetRenderTexture(renderTexture);
#endif
	
#ifdef PARTICLE_SYSTEM
	ref_ptr<GameObject> go = new GameObject();
	ParticleSystem* particleSystem = go->AddComponent<ParticleSystem>();
	go->GetTransform()->SetPosition(Vector3(-30, 20, -50));
	go->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<SphereParticleEmitter> emitter = new SphereParticleEmitter();
	emitter->SetRadius(5);
	emitter->SetRate(200);
	emitter->SetStartColor(Vector4(1, 1, 1, 0.5f));
	emitter->SetStartDuration(2);
	emitter->SetStartSize(1);
	emitter->SetStartVelocity(Vector3(0, 1, 0));
	ParticleBurst burst = { 4, 3, 20 };
	particleSystem->SetEmitter(emitter.get());

	ref_ptr<ParticleAnimator> animator = new ParticleAnimator();
	animator->SetGravityScale(0.2f);
	particleSystem->SetParticleAnimator(animator.get());

	particleSystem->SetMaxParticles(1000);
	particleSystem->SetDuration(5);
	particleSystem->SetLooping(true);
#endif

#if defined(FONT)
	Font font = new IFont();
	font->Load("fonts/ms_yh.ttf", 12);

	GameObject* redText = new GameObject();
	redText->SetName("RedText");
	redText->GetTransform()->SetPosition(Vector3(-10, 20, -20));
	redText->GetTransform()->SetParent(Engine::GetRootTransform());

	GameObject* blueText = new GameObject();
	blueText->SetName("BlueText");
	blueText->GetTransform()->SetPosition(Vector3(-10, 30, -20));
	blueText->GetTransform()->SetParent(Engine::GetRootTransform());

	TextMesh redMesh = redText->AddComponent<TextMesh>();
	redMesh->SetFont(font);
	redMesh->SetText("落霞与孤鹜齐飞");
	redMesh->SetFontSize(12);

	TextMesh blueMesh = blueText->AddComponent<TextMesh>();
	blueMesh->SetFont(font);
	blueMesh->SetText("秋水共长天一色");
	blueMesh->SetFontSize(12);

	Renderer redRenderer = redText->AddComponent<MeshRenderer>();
	Material redMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	redMaterial->SetColor(BuiltinProperties::MainColor, Color(1, 0, 0, 1));
	redRenderer->AddMaterial(redMaterial);

	Renderer blueRenderer = blueText->AddComponent<MeshRenderer>();
	Material blueMaterial = suede_dynamic_cast<Material>(font->GetMaterial()->Clone());
	blueMaterial->SetColor(BuiltinProperties::MainColor, Color(0, 0, 1, 1));
	blueRenderer->AddMaterial(blueMaterial);

#endif

	auto gameObjectImported = [this](GameObject* go, const std::string& path) {
		onGameObjectImported(go, path);
	};

#ifdef ROOM
	GameObject* room = scene->Import(roomFbxPath, gameObjectImported);
#endif

#ifdef BUMPED
	GameObject* bumped = scene->Import(bumpedFbxPath, gameObjectImported);
#endif

#ifdef NORMAL_VISUALIZER
	GameObject* normalVisualizer = scene->Import(normalVisualizerFbxPath, gameObjectImported);
#endif

#ifdef BEAR
	GameObject* bear = scene->Import("teddy_bear.fbx", gameObjectImported);
	bear->GetTransform()->SetPosition(Vector3(0, -20, -150));
#ifdef BEAR_X_RAY
	Material materail = bear->FindChild("Teddy_Bear")->GetRenderer()->GetMaterial(0);
	Shader shader = Shader::FindShader("xray");
	materail->SetShader(shader);
#endif

#endif

#ifdef ANIMATION
	GameObject* man = scene->Import(manFbxPath, gameObjectImported);
#endif
}
