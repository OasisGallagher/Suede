SuedeGlobal = { }

function SuedeGlobal.Awake()
end

function SuedeGlobal.Start()
	Suede.WorldInstance():CullingUpdate();
	print(Suede.CameraUtility);
	Suede.CameraUtility.GetMain();

	Suede.WorldInstance():Import("suzanne.fbx", function (root, path)
		print("loaded " .. path);
		root:GetTransform():SetEulerAngles({ });
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
