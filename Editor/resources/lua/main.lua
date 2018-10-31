SuedeGlobal = { }

function SuedeGlobal.Awake()
end

function SuedeGlobal.Start()
	Suede.EnvironmentInstance():SetFogColor(Suede.NewColor(0.5, 0.5, 0.5));
	Suede.EnvironmentInstance():SetFogDensity(0);
	Suede.EnvironmentInstance():SetAmbientColor(Suede.NewColor(0.15, 0.15, 0.15));

	Suede.WorldInstance():Import("suzanne.fbx", function (root, path)
		print("loaded " .. path);
		root:GetTransform():SetEulerAngles({ Suede.DepthTextureMode.Depth, Suede.MeshTopology.TriangleStripe });
		root:GetTransform():SetPosition({ 0, 25, -5 });
		root:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());

		local diffuse = Suede.NewTexture2D();
		diffuse:Create("suzanne/diffuse.dds");
		local target = root:GetTransform():FindChild("suzanne_root/default"):GetGameObject();

		local material = target:GetComponent("IMeshRenderer"):GetMaterial(0);
		material:SetTexture("_MainTexture", diffuse);
	end);
end

function SuedeGlobal.Update()
end
