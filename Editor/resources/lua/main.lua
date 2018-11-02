SuedeGlobal = { }

function SuedeGlobal.Awake()
end

function SuedeGlobal.Start()
	Suede.Environment.SetFogColor(Suede.NewColor(0.5, 0.5, 0.5));
	Suede.Environment.SetFogDensity(0);
	Suede.Environment.SetAmbientColor(Suede.NewColor(0.15, 0.15, 0.15));

	Suede.World.Import("suzanne.fbx", function (root, path)
		print("loaded " .. path);
		root:GetTransform():SetEulerAngles({ Suede.DepthTextureMode.Depth, Suede.MeshTopology.TriangleStripe });
		root:GetTransform():SetPosition({ 0, 25, -5 });
		root:GetTransform():SetParent(Suede.World.GetRootTransform());

		local diffuse = Suede.NewTexture2D();
		diffuse:Create("suzanne/diffuse.dds");
		local target = root:GetTransform():FindChild("suzanne_root/default"):GetGameObject();

		local material = target:GetComponent("IMeshRenderer"):GetMaterial(0);
		material:SetTexture("_MainTexture", diffuse);
	end);
end

function SuedeGlobal.Update()
	if Suede.Input.GetKeyUp(Suede.KeyCode.Space) then
		print("Space key up");
	end
end
