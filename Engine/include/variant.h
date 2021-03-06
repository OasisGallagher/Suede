#pragma once
#include "color.h"
#include "ranged.h"
#include "texture.h"
#include "math/matrix4.h"

BETTER_ENUM(VariantType, int,
	None,
	Int,
	Bool,
	Float,
	RangedInt,
	RangedFloat,
	Matrix3,
	Matrix4,
	IVector3,
	Vector2,
	Vector3,
	Color,
	Vector4,
	Texture,
	Quaternion,

	// POD array.
	_POD_ARRAY_BEGIN,
	String = _POD_ARRAY_BEGIN,
	Vector3Array,
	Matrix4Array
)

class SUEDE_API Variant {
public:
	Variant() : type_(VariantType::None) { }
	Variant(int value) { SetInt(value); }
	Variant(bool value) { SetBool(value); }
	Variant(float value) { SetFloat(value); }
	Variant(const iranged& value) { SetRangedInt(value); }
	Variant(const franged& value) { SetRangedFloat(value); }
	Variant(const Matrix4& value) { SetMatrix4(value); }
	Variant(const Vector2& value) { SetVector2(value); }
	Variant(const Vector3& value) { SetVector3(value); }
	Variant(const IVector3& value) { SetIVector3(value); }
	Variant(const Color& value) { SetColor(value); }
	Variant(const Vector4& value) { SetVector4(value); }
	Variant(const Quaternion& value) { SetQuaternion(value); }
	Variant(const std::string& value) { SetString(value); }
	Variant(const Vector3* data, uint size) { SetVector3Array(data, size); }
	Variant(const Matrix4* data, uint size) { SetMatrix4Array(data, size); }
	Variant(Texture* value) { SetTexture(value); }

	Variant(Variant&& other);
	Variant(const Variant& other);
	~Variant();

public:
	int GetInt() const;
	bool GetBool() const;
	float GetFloat() const;
	iranged GetRangedInt() const;
	franged GetRangedFloat() const;
	Matrix4 GetMatrix4() const;
	Vector2 GetVector2() const;
	Vector3 GetVector3() const;
	IVector3 GetIVector3() const;
	Color GetColor() const;
	Vector4 GetVector4() const;
	Quaternion GetQuaternion() const;

	std::string GetString() const;
	
	const Vector3* GetVector3Array() const;
	uint GetVector3ArraySize() const;
	
	const Matrix4* GetMatrix4Array() const;
	uint GetMatrix4ArraySize() const;
	Texture* GetTexture() const;

	VariantType GetType() const { return type_; }

	void SetInt(int value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetRangedInt(const iranged& value);
	void SetRangedFloat(const franged& value);
	void SetMatrix4(const Matrix4& value);
	void SetVector2(const Vector2& value);
	void SetVector3(const Vector3& value);
	void SetIVector3(const IVector3& value);
	void SetColor(const Color& value);
	void SetVector4(const Vector4& value);
	void SetQuaternion(const Quaternion& value);
	void SetString(const std::string& value);
	void SetVector3Array(const Vector3* data, uint size);
	void SetMatrix4Array(const Matrix4* data, uint size);
	void SetTexture(Texture* value);

	const void* GetData() const;
	uint GetDataSize() const;

public:
	Variant& operator = (const Variant& other);
	bool operator == (const Variant& other) const;

private:
	bool SetType(VariantType type);
	void SetPodArray(VariantType type, const void* data, uint size);

private:
	union Data { // pod only.
		Data() {}

		int intValue;
		bool boolValue;
		float floatValue;
		Matrix4 mat4Value;
		IVector3 ivec3Value;
		Vector2 vec2Value;
		Vector3 vec3Value;
		Vector4 vec4Value;
		Quaternion quatValue;
		Color colorValue;
		struct {
			char* ptr;
			uint size;
		} podArray;
	} data_;

	// non-pod data.
	ref_ptr<Texture> texture_;

	VariantType type_;
};
