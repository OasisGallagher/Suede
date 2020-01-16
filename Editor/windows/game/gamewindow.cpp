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

#include "main/selection.h"
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
//#define ANIMATION
//#define PARTICLE_SYSTEM
//#define FONT
//#define BUMPED
//#define NORMAL_VISUALIZER
//#define DEFERRED_RENDERING

static const char* roomFbxPath = "house.fbx";
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
	
	connect(ui_->showGizmos, &QPushButton::clicked, this, &GameWindow::onToggleGizmos);
	connect(ui_->drawPhysics, &QPushButton::clicked, this, &GameWindow::onToggleDrawPhysics);
	connect(ui_->showStatistics, &QPushButton::clicked, this, &GameWindow::onToggleStatistics);
	connect(editor_->childWindow<HierarchyWindow>(), &HierarchyWindow::focusGameObject, this, &GameWindow::onFocusGameObjectBounds);
	
	typedef void (EnumField::*fptr)(const QString&);
	connect(ui_->shadingMode, (fptr)&EnumField::currentIndexChanged, this, &GameWindow::onShadingModeChanged);
}

void GameWindow::awake() {
	Component::Register<CameraController>();

	ui_->showGizmos->setChecked(true);
	ui_->shadingMode->setEnums(Engine::GetSubsystem<Graphics>()->GetShadingMode());
	ui_->drawPhysics->setChecked(Engine::GetSubsystem<Physics>()->GetDebugDrawEnabled());

	input_ = Engine::GetSubsystem<Input>();

	inputDelegate_ = new QtInputDelegate(canvas_);
	input_->SetDelegate(inputDelegate_);

	setupScene();
}

void GameWindow::tick() {
	if (input_->GetMouseButtonUp(0)) {
		RaycastHit hitInfo;
		Vector3 src = Camera::GetMain()->GetTransform()->GetPosition();
		Vector2 mousePosition = input_->GetMousePosition();
		Vector3 dest = Camera::GetMain()->ScreenToWorldPoint(Vector3(mousePosition.x, mousePosition.y, 1));

		editor_->selection()->clear();

		if (Engine::GetSubsystem<Physics>()->Raycast(Ray(src, dest - src), 1000, &hitInfo)) {
			editor_->selection()->add(hitInfo.gameObject);
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

void GameWindow::onToggleGizmos(bool checked) {
	selectionGizmos_->SetEnabled(checked);
}

void GameWindow::onToggleStatistics(bool checked) {
	stat_->setVisible(checked);

	if (stat_->isVisible()) {
		updateStatPosition();
	}
}

void GameWindow::onToggleDrawPhysics(bool checked) {
	Engine::GetSubsystem<Physics>()->SetDebugDrawEnabled(checked);
}

void GameWindow::updateStatPosition() {
	QPoint pos = ui_->showStatistics->parentWidget()->mapTo(this, ui_->showStatistics->pos());
	pos.setX(pos.x() - stat_->width());
	pos.setY(pos.y() + ui_->showStatistics->height() + 2);
	stat_->move(pos);
}

void GameWindow::onShadingModeChanged(const QString& str) {
	Engine::GetSubsystem<Graphics>()->SetShadingMode(ShadingMode::from_string(str.toLatin1()));
}

void GameWindow::onFocusGameObjectBounds(GameObject* go) {
	Bounds bounds;
	for (Renderer* renderer : go->GetComponentsInChildren<Renderer>()) {
		bounds.Encapsulate(renderer->GetBounds());
	}

	Transform* target = go->GetTransform();
	Transform* camera = Camera::GetMain()->GetTransform();
	float distance = calculateCameraDistanceFitsBounds(Camera::GetMain(), bounds);
	camera->SetPosition(bounds.center + target->GetForward() * distance);

	Quaternion q(Matrix4::LookAt(camera->GetPosition(), bounds.center, Vector3::up));
	camera->SetRotation(q.GetInversed());
}

float GameWindow::calculateCameraDistanceFitsBounds(Camera* camera, const Bounds& bounds) {
	float f = tanf(camera->GetFieldOfView() / 2.f);
	float dy = bounds.size.y / f;
	float dx = bounds.size.x / (f * camera->GetAspect());
	return Mathf::Clamp(Mathf::Min(dx, dy), camera->GetNearClipPlane() + 0.01f, camera->GetFarClipPlane() - 0.01f) + Mathf::Max(bounds.size.x, bounds.size.y) / 2.f;
}

void GameWindow::setupScene() {
	Scene* scene = Engine::GetSubsystem<Scene>();

	ref_ptr<GameObject> lightGameObject = new GameObject();
	lightGameObject->SetName("light");

	Light* light = lightGameObject->AddComponent<Light>();
	light->SetColor(Color::white);
	light->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<GameObject> cameraGameObject = new GameObject();
	cameraGameObject->SetName("camera");

	Camera* camera = cameraGameObject->AddComponent<Camera>();
	Camera::SetMain(camera);
	camera->SetDepthTextureMode(DepthTextureMode::Depth);
	camera->GetTransform()->SetParent(scene->GetRootTransform());

	controller_ = cameraGameObject->AddComponent<CameraController>();
	controller_->setView(this);
	controller_->setSelection(editor_->selection());

	selectionGizmos_ = cameraGameObject->AddComponent<SelectionGizmos>();
	selectionGizmos_->setSelection(editor_->selection());

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
	ref_ptr<GameObject> go = new GameObject("ParticleSystem");
	ParticleSystem* particleSystem = go->AddComponent<ParticleSystem>();
	go->GetTransform()->SetPosition(Vector3(-30, 20, -50));
	go->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<SphereParticleEmitter> emitter = new SphereParticleEmitter();
	emitter->SetRadius(5);
	emitter->SetRate(200);
	emitter->SetStartColor(Vector4(1, 1, 1, 0.5f));
	emitter->SetStartDuration(2);
	emitter->SetStartSize(1);
	emitter->SetStartVelocity(Vector3::up);
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
	ref_ptr<Font> font = new Font();
	font->Load("fonts/ms_yh.ttf");

	ref_ptr<GameObject> redText = new GameObject();
	redText->SetName("RedText");
	redText->GetTransform()->SetPosition(Vector3(-10, 20, -200));
	redText->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<GameObject> yellowText = new GameObject();
	yellowText->SetName("BlueText");
	yellowText->GetTransform()->SetPosition(Vector3(-10, 40, -200));
	yellowText->GetTransform()->SetParent(scene->GetRootTransform());

	ref_ptr<TextMesh> redMesh = redText->AddComponent<TextMesh>();
	redMesh->SetFont(font.get());
	redMesh->SetText(L"落霞与孤鹜齐飞abcdefgW");
	redMesh->SetFontSize(12);

	ref_ptr<TextMesh> yellowMesh = yellowText->AddComponent<TextMesh>();
	yellowMesh->SetFont(font.get());
	yellowMesh->SetText(L"秋水共长天一色hijklmnQ");
	yellowMesh->SetFontSize(12);

	ref_ptr<Renderer> redRenderer = redText->AddComponent<MeshRenderer>();
	ref_ptr<Material> redMaterial = dynamic_ref_ptr_cast<Material>(font->GetMaterial()->Clone());
	redMaterial->SetColor(BuiltinProperties::MainColor, Color::red);
	redRenderer->AddMaterial(redMaterial.get());

	ref_ptr<Renderer> yellowRenderer = yellowText->AddComponent<MeshRenderer>();
	ref_ptr<Material> yellowMaterial = dynamic_ref_ptr_cast<Material>(font->GetMaterial()->Clone());
	yellowMaterial->SetColor(BuiltinProperties::MainColor, Color::yellow);
	yellowRenderer->AddMaterial(yellowMaterial.get());

#endif

	auto gameObjectImported = [this](GameObject* go, const std::string& path) {
		onGameObjectImported(go, path);
	};

#ifdef ROOM
	scene->Import(roomFbxPath, gameObjectImported);
#endif

#ifdef BUMPED
	scene->Import(bumpedFbxPath, gameObjectImported);
#endif

#ifdef NORMAL_VISUALIZER
	GameObject* normalVisualizer = scene->Import(normalVisualizerFbxPath, gameObjectImported);
#endif

#if defined(BEAR) || defined(BEAR_X_RAY)
	scene->Import("nanosuit.fbx", [this](GameObject* go, const std::string&) {
		if (!go) { return; }
		go->GetTransform()->SetPosition(Vector3(0, -20, -150));
#ifdef BEAR_X_RAY
		/*ref_ptr<Material> materail = go->GetTransform()->FindChild("Teddy_Bear")->GetGameObject()->GetComponent<Renderer>()->GetMaterial(0);
		materail->SetShader(Shader::Find("xray"));*/
#endif
});

#endif

#ifdef ANIMATION
	scene->Import(manFbxPath, gameObjectImported);
#endif
}
