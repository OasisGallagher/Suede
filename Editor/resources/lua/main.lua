SuedeGlobal = { }

function SuedeGlobal.Awake()
end

function SuedeGlobal.Start()
	local color = Suede.NewColor();
	color:Set(0.5, 0.5, 0.5);
	Suede.EnvironmentInstance():SetFogColor(color);
	Suede.EnvironmentInstance():SetFogDensity(0);

	color:Set(0.15, 0.15, 0.15);
	Suede.EnvironmentInstance():SetAmbientColor(color);

	Suede.WorldInstance():Import("suzanne.fbx", function (root, path)
		print("loaded " .. path);
		root:GetTransform():SetEulerAngles({ Suede.DepthTextureMode.Depth, Suede.MeshTopology.TriangleStripe });
		root:GetTransform():SetPosition({ 0, 25, -5 });
		root:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());

		local diffuse = Suede.NewTexture2D();
		diffuse:Create("suzanne/diffuse.dds");
		local target = root:GetTransform():FindChild("suzanne_root/default"):GetGameObject();

		local renderer = target:GetComponent("IMeshRenderer");
		print(renderer);
		local material = renderer:GetMaterial(0);
		material:SetTexture("_MainTexture", diffuse);
	end);
end

function SuedeGlobal.Update()
end
