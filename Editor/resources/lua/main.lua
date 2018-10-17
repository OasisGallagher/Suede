SuedeGlobal = {

}

SuedeGlobal.update = function ()
    if Suede.TimeInstance():GetFrameCount() > 10 and not SuedeGlobal.gameObject then
        SuedeGlobal.gameObject = Suede.NewGameObject();
		SuedeGlobal.gameObject:SetName("LuaGameObject");
		SuedeGlobal.gameObject:GetTransform():SetParent(Suede.WorldInstance():GetRootTransform());
        print("instance id = " .. SuedeGlobal.gameObject:GetInstanceID());
    end
	--print("Update SuedeGlobal, elapsed = " .. Suede.TimeInstance():GetDeltaTime());
end
