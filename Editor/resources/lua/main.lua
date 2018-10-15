SuedeGlobal = {

}

SuedeGlobal.update = function ()
    if not SuedeGlobal.gameObject then
        SuedeGlobal.gameObject = Suede.NewGameObject();
        SuedeGlobal.gameObject:GetInstanceID();
    end
	--print("Update SuedeGlobal, elapsed = " .. Suede.TimeInstance():GetDeltaTime());
end
