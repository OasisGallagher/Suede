SuedeGlobal = { }

function SuedeGlobal.Awake()
end

function SuedeGlobal.Start()
	Suede.Graphics.SetAmbientOcclusionEnabled(false);

	Suede.Environment.SetFogColor(Suede.NewColor(0.5, 0.5, 0.5));
	Suede.Environment.SetFogDensity(0);
	Suede.Environment.SetAmbientColor(Suede.NewColor(0.15, 0.15, 0.15));

	Suede.World.Import("house.fbx", function (root, path)
		--root:GetTransform():SetPosition({ 0, 25, -65 });
		--root:GetTransform():SetEulerAngles({ 30, 0, 0 });

		local body = root:GetTransform():FindChild("Sphere"):GetGameObject():GetComponent("IRigidbody");
		body:SetMass(0);

		if string.find(path, "house") then
			root:GetTransform():SetScale({ 0.01, 0.01, 0.01 });
		end
	end);

	--[[
	Suede.World.Import("builtin/quad.fbx", function (root, path)
		local target = root:GetTransform():FindChild("quad_root/default"):GetGameObject();
		root:GetTransform():SetEulerAngles({ 0, 180, 0 });
		
		local shader = Suede.NewShader();
		shader:Load("builtin/lit_parallactic_texture");

		local material = target:GetComponent("IMeshRenderer"):GetMaterial(0);
		material:SetShader(shader);

		local diffuse = Suede.NewTexture2D();
		diffuse:Load("parallax/diffuse.jpg");

		local normal = Suede.NewTexture2D();
		normal:Load("parallax/normal.jpg");

		local depth = Suede.NewTexture2D();
		depth:Load("parallax/depth.jpg");

		material:SetTexture("_MainTexture", diffuse);
		material:SetTexture("_BumpTexture", normal);
		material:SetTexture("_DepthTexture", depth);
	end);
	]]

	--[[
	Suede.World.Import("suzanne.fbx", function (root, path)
		print("loaded " .. path);
		root:GetTransform():SetPosition({ 0, 25, -5 });

		local diffuse = Suede.NewTexture2D();
		diffuse:Load("suzanne/diffuse.dds");
		local target = root:GetTransform():FindChild("suzanne_root/default"):GetGameObject();

		local material = target:GetComponent("IMeshRenderer"):GetMaterial(0);
		material:SetTexture("_MainTexture", diffuse);
	end);
	]]
end

function SuedeGlobal.Update()
	if Suede.Input.GetKeyUp(Suede.KeyCode.Space) then
		print("Space key up");
	end
end
