# Suede
## ScreenShots
![Suede Overview][overview]
## Projects
- **Editor** 基于Qt5的编辑器界面。
  - *Game:* 图形显示窗口，可使用GUI接口进行Immediate模式的UI绘制。
  - *Hierarchy:* 场景内GameObject显示层级，支持拖拽节点改变结构，拖拽模型文件加载模型。
  - *Project:* 工程目录窗口，支持文件搜索，打开，显示所在文件夹等常用功能。
  - *Inspector:* 使用ImGUI进行渲染。可实时显示/修改选择对象所包含的Component的属性，对象的材质信息。
  - *Console:* 显示/过滤/查找日志信息。
  - *Lighting:* 环境光，闭塞等光照相关信息配置。
- **Engine** 基于OpenGL的渲染引擎，对OpenGL底层接口进行C++封装。
  - 主线程渲染，工作线程通过AABB拣选。
  - 支持Mesh, Texture，Shader，Camera，Material等常用类，隔离OpenGL底层接口。
  - 支持类似ShaderLab的语法，可以在单个shader文件中，完成大部分渲染配置工作。
  - 使用模块化的组件模式，可以方便的通过名称，GUID，类型来增删改查GameObject上的组件。
  - 支持Lua作为脚本语言并自动生成转换文件。可访问引擎对象，注册回调函数。支持重载函数，枚举等。
  - 支持文字，Decal，简单的骨骼动画和粒子效果。
- **GLEF** Shader语法解析器。
  - 使用LALR进行语法分析，构造语法树。
  - 语法与解析过程分离，使语法可独立修改。
- **GLEFBuilder** 对比文件修改时间，在编译器自动构建GLEF解析器。
- **Shared** 一些常用工具，容器，内存分配，Log等。
## Examples
**加载图片**

C++
```c++
Texture2D diffuse = NewTexture2D();

// 图片加载以resources/textures为根目录。
bool status = diffuse->Create("suzanne/diffuse.dds");

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
```
Lua
```lua
local diffuse = Suede.NewTexture2D();

-- 图片加载以resources/textures为根目录。
diffuse:Create("suzanne/diffuse.dds");

local cube = Suede.NewTextureCube();

local faces = {
    "lake_skybox/right.jpg",
    "lake_skybox/left.jpg",
    "lake_skybox/top.jpg",
    "lake_skybox/bottom.jpg",
    "lake_skybox/back.jpg",
    "lake_skybox/front.jpg",
};

cube:Load(faces);
```

**加载模型**

C++
```c++
// 模型加载以resources/models为根目录。
World::instance()->Import("suzanne.fbx", this);

// 继承GameObjectImportedListener, 重写回调函数。
void Scene::OnGameObjectImported(GameObject root, const std::string& path) {
    // !!! 将GameObject添加到场景中。.
    root->GetTransform()->SetParent(World::instance()->GetRootTransform());
}
```
Lua
```lua
-- 模型加载以resources/models为根目录。
Suede.WorldInstance():Import("suzanne.fbx", function (root, path)
    -- !!! 将GameObject添加到场景中。.
    root:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());
end);
```
**创建组件**

C++
```c++
GameObject cameraGameObject = NewGameObject();
cameraGameObject->SetName("camera");

Camera camera = cameraGameObject->AddComponent<ICamera>();
CameraUtility::SetMain(camera);

// !!! 将GameObject添加到场景中。.
camera->GetTransform()->SetParent(World::instance()->GetRootTransform());
```
Lua
```lua
local cameraGameObject = Suede.NewGameObject();
cameraGameObject:SetName("camera");

local camera = cameraGameObject:AddComponent("ICamera");

-- 调用静态成员函数。
Suede.CameraUtility.SetMain(camera);

-- !!! 将GameObject添加到场景中。.
camera:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());
```
**替换/修改材质**

C++
```c++
GameObject target = root->GetTransform()->FindChild("Sphere01")->GetGameObject();
Material material = target->GetComponent<IMeshRenderer>()->GetMaterial(0);

// Shader加载以resources/shaders为根目录。
material->SetShader(Resources::instance()->FindShader("builtin/lit_bumped_texture"));

Texture2D diffuse = NewTexture2D();
diffuse->Create("bumped/diffuse.jpg");
material->SetTexture(BuiltinProperties::MainTexture, diffuse);
```
Lua
```lua
local target = root:GetTransform():FindChild("Sphere01"):GetGameObject();
local material = target:GetComponent("IMeshRenderer"):GetMaterial(0);

-- Shader加载以resources/shaders为根目录。
material:SetShader(Suede.ResourcesInstance():FindShader("builtin/lit_bumped_texture"));

local diffuse = Suede.NewTexture2D();
diffuse:Create("bumped/diffuse.jpg");
material:SetTexture("_MainTexture", diffuse);
```
**自定义组件（只支持C++定义组件）**
```c++
// header
class CameraController : public QObject, public Behaviour {
    Q_OBJECT
    
    // 下列属性，可以在Inspector中查看并修改.
    Q_PROPERTY(glm::vec3 MoveSpeed READ moveSpeed WRITE setMoveSpeed)
    Q_PROPERTY(glm::vec2 OrientSpeed READ orientSpeed WRITE setOrientSpeed)
    Q_PROPERTY(glm::vec2 RotateSpeed READ rotateSpeed WRITE setRotateSpeed)

    SUEDE_DECLARE_COMPONENT()
    ...
    
public:
    virtual void Awake() { }    // 创建成功调用。
    virtual void Update() { }    // 每一帧调用。
};

// source
SUEDE_DEFINE_COMPONENT(CameraController, Behaviour)
```
**创建Shader**
```c++
Properties {
    // 颜色属性, 与下面的uniform vec4 _MainColor对应。
    color _MainColor = { 1, 1, 1, 1 };
}

SubShader {
    // 如果选中的SubShader含有多个Pass，渲染时会逐个执行。
    Pass "PassName" true {    // 可以指定名称和是否启用，也可以忽略，默认为启用。
        // 设置渲染状态，该Pass结束之后，会重置这些状态。
        Cull Front;
        ZTest LEqual;

        // 开始GLSL代码块。
        GLSLPROGRAM

        // 截止到下一个#stage标签之前，为顶点着色器。
        #stage vertex
        
        // 包含文件，以resources/shaders目录为根目录。
        #include "builtin/include/suede.inc"

        in vec3 _Pos;
        out vec3 texCoord;

        void main() {
            vec4 pos = _CameraToClipMatrix * vec4(mat3(_WorldToCameraMatrix) * _Pos, 1);
            gl_Position = pos.xyww;
            texCoord = _Pos;
        }

        #stage fragment

        in vec3 texCoord;
        out vec4 fragColor;

        uniform vec4 _MainColor;
        uniform samplerCube _MainTexture;

        void main() {
            fragColor = texture(_MainTexture, texCoord) * _MainColor;
        }

        ENDGLSL
    }
}
```
## Dependencies
- *Lua* 引擎脚本语言。
- *Python* 生成C++ Lua Wrappers。
- *Glew* 加载OpenGL API。
- *Assimp* 加载模型，材质。
- *FreeType* 加载字体。
- *FreeImage* 加载图片。
- *ZThread* Engine中的线程。
- *better-enum* 枚举与字符串转换，遍历枚举内容。
- *Visual Leak Detector* 检查内存泄漏。

[overview]: ScreenShots/overview.png
