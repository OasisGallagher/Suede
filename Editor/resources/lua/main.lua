_G.printf = function (...)
	print(string.format(...));
end

SuedeGlobal = { }

function SuedeGlobal.Awake()
	print("lua Awake");
	local go = Suede.NewGameObject();
	go:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());
	go:__set_callback_function(function()
		print("callback function called");
	end)

	go:SetName("luaGameObject");
end

function SuedeGlobal.Update()
end
