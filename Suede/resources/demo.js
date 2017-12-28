Properties {
	int x = 4;
	tex2 main = { 255, 0, 0 };
}

SubShader {
	Tags {
		Queue = "opaque";
		Queue = "geometry";
		Queue = "transparent";
	}
	
	Pass {
		Cull Off;
		
		GLPROGRAM
		
		#include "main.inc"
		
		ENDGL
	}
	
	Pass {
	}
	
	Pass {
	}
}

SubShader {
	Tags {
		Queue = "opaque";
		Queue = "geometry";
		Queue = "transparent";
	}
	
	Pass {
		Cull Off;
		Blend SrcAlpha OneMinusSrcAlpha;
		
		GLPROGRAM
		#include "main.inc"
		ENDGL
	}
}

SubShader {
	Pass {
	}
}

