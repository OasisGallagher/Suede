SuedeGlobal = { }

function SuedeGlobal.Awake()
	print("lua Awake");
	local go = Suede.NewGameObject();
	go:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());
	go:SetName("luaGameObject");
end

function SuedeGlobal.Start()
	print("lua Start");
	local f = function (go, path)
		print("loaded " .. path);
		go:GetTransform():SetEulerAngles({ 30, 0, 0 });
		go:GetTransform():SetPosition({ 0, 25, -65 });
		go:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());	
	end

	Suede.WorldInstance():Import("room.fbx", f);
end

function SuedeGlobal.Update()
end
