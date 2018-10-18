_G.printf = function (...)
	print(string.format(...));
end

SuedeGlobal = { }

function SuedeGlobal.Awak2e()
	print("lua Awake");
	local go = Suede.NewGameObject();
	go:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());

	go:SetName("luaGameObject");
end

function SuedeGlobal.Update()
end
