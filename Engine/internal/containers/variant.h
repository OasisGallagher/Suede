#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "texture.h"

enum VariantType {
	VariantTypeNone,
	VariantTypeInt,
	VariantTypeBool,
	VariantTypeFloat,
	VariantTypeMatrix4,
	VariantTypeVector3,
	VariantTypeVector4,
	VariantTypeTexture,
	VariantTypePodBuffer,
	VariantTypeQuaternion,
};

class Variant {
public:
	Variant() :type_(VariantTypeNone) {
	}
	~Variant();

public:
	static const char* TypeString(VariantType type);

public:
	int GetInt();
	bool GetBool();
	float GetFloat();
	glm::mat4 GetMatrix4();
	glm::vec3 GetVector3();
	glm::vec4 GetVector4();
	glm::quat GetQuaternion();
	void* GetPodBuffer();
	uint GetPodBufferSize();
	Texture GetTexture();
	int GetTextureIndex();

	VariantType GetType() { return type_; }

	void SetInt(int value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetMatrix4(const glm::mat4& value);
	void SetVector3(const glm::vec3& value);
	void SetVector4(const glm::vec4& value);
	void SetQuaternion(const glm::quat& value);
	void SetPodBuffer(const void* data, uint size);
	void SetTexture(Texture value);
	void SetTextureIndex(GLenum value);

	const void* GetData() const;

private:
	bool SetType(VariantType type);

private:
	union Data { // pod only.
		Data() {}

		int intValue;
		bool boolValue;
		float floatValue;
		glm::mat4 mat4Value;
		glm::vec3 vec3Value;
		glm::vec4 vec4Value;
		glm::quat quatValue;
		int textureIndex;
		struct {
			char* ptr;
			uint size;
		} podBuffer;
	} data_;

	// non-pod data.
	Texture texture_;
	
	VariantType type_;
};
