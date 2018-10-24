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
		go:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());	
	end

	Suede.WorldInstance():Import("room.fbx", f);
end

function SuedeGlobal.Update()
end
