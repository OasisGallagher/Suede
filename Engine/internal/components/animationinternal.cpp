#include "animationinternal.h"

#include <set>
#include <algorithm>

#include "time2.h"
#include "math/mathf.h"

Skeleton::Skeleton() : Object(new SkeletonInternal) { }
bool Skeleton::AddBone(const SkeletonBone& bone) { return _suede_dptr()->AddBone(bone); }
SkeletonBone* Skeleton::GetBone(uint index) { return _suede_dptr()->GetBone(index); }
SkeletonBone* Skeleton::GetBone(const std::string& name) { return _suede_dptr()->GetBone(name); }
SkeletonNode* Skeleton::CreateNode(const std::string& name, const Matrix4& matrix, AnimationCurve* curve) { return _suede_dptr()->CreateNode(name, matrix, curve); }
void Skeleton::AddNode(SkeletonNode* parent, SkeletonNode* child) { return _suede_dptr()->AddNode(parent, child); }
SkeletonNode* Skeleton::GetRootNode() { return _suede_dptr()->GetRootNode(); }
void Skeleton::SetBoneToRootMatrix(uint index, const Matrix4& value) { return _suede_dptr()->SetBoneToRootMatrix(index, value); }
Matrix4* Skeleton::GetBoneToRootMatrices() { return _suede_dptr()->GetBoneToRootMatrices(); }
int Skeleton::GetBoneIndex(const std::string& name) { return _suede_dptr()->GetBoneIndex(name); }
int Skeleton::GetBoneCount() { return _suede_dptr()->GetBoneCount(); }

AnimationClip::AnimationClip() :Object(new AnimationClipInternal) {}
void AnimationClip::SetWrapMode(AnimationWrapMode value) { _suede_dptr()->SetWrapMode(value); }
AnimationWrapMode AnimationClip::GetWrapMode() { return _suede_dptr()->GetWrapMode(); }
void AnimationClip::SetTicksPerSecond(float value) { _suede_dptr()->SetTicksPerSecond(value); }
float AnimationClip::GetTicksPerSecond() { return _suede_dptr()->GetTicksPerSecond(); }
void AnimationClip::SetDuration(float value) { _suede_dptr()->SetDuration(value); }
float AnimationClip::GetDuration() { return _suede_dptr()->GetDuration(); }
void AnimationClip::SetAnimation(Animation* value) { _suede_dptr()->SetAnimation(value); }
Animation* AnimationClip::GetAnimation() { return _suede_dptr()->GetAnimation(); }
bool AnimationClip::Sample(float time) { return _suede_dptr()->Sample(time); }

AnimationState::AnimationState() : Object(new AnimationStateInternal) {}

AnimationKeys::AnimationKeys() : Object(new AnimationKeysInternal) {}
void AnimationKeys::AddFloat(float time, int id, float value) { _suede_dptr()->AddFloat(time, id, value); }
void AnimationKeys::AddVector3(float time, int id, const Vector3& value) { _suede_dptr()->AddVector3(time, id, value); }
void AnimationKeys::AddQuaternion(float time, int id, const Quaternion& value) { _suede_dptr()->AddQuaternion(time, id, value); }
void AnimationKeys::Remove(float time, int id) { _suede_dptr()->Remove(time, id); }
void AnimationKeys::ToKeyframes(std::vector<ref_ptr<AnimationFrame>>& keyframes) { _suede_dptr()->ToKeyframes(keyframes); }

AnimationFrame::AnimationFrame() : Object(new AnimationFrameInternal) {}
void AnimationFrame::SetTime(float value) { _suede_dptr()->SetTime(value); }
float AnimationFrame::GetTime() { return _suede_dptr()->GetTime(); }
void AnimationFrame::Assign(AnimationFrame* other) { _suede_dptr()->Assign(other); }
void AnimationFrame::Lerp(AnimationFrame* result, AnimationFrame* other, float factor) { _suede_dptr()->Lerp(result, other, factor); }
void AnimationFrame::SetFloat(int id, float value) { _suede_dptr()->SetFloat(id, value); }
void AnimationFrame::SetVector3(int id, const Vector3& value) { _suede_dptr()->SetVector3(id, value); }
void AnimationFrame::SetQuaternion(int id, const Quaternion& value) { _suede_dptr()->SetQuaternion(id, value); }
float AnimationFrame::GetFloat(int id) { return _suede_dptr()->GetFloat(id); }
Vector3 AnimationFrame::GetVector3(int id) { return _suede_dptr()->GetVector3(id); }
Quaternion AnimationFrame::GetQuaternion(int id) { return _suede_dptr()->GetQuaternion(id); }

AnimationCurve::AnimationCurve() : Object(new AnimationCurveInternal) {}
void AnimationCurve::SetKeys(AnimationKeys* value) { _suede_dptr()->SetKeys(value); }
bool AnimationCurve::Sample(float time, AnimationFrame*& frame) { return _suede_dptr()->Sample(time, frame); }

Animation::Animation() : Component(new AnimationInternal) {}
void Animation::AddClip(const std::string& name, AnimationClip* value) { _suede_dptr()->AddClip(this, name, value); }
AnimationClip* Animation::GetClip(const std::string& name) { return _suede_dptr()->GetClip(name); }
void Animation::SetSkeleton(Skeleton* value) { _suede_dptr()->SetSkeleton(value); }
Skeleton* Animation::GetSkeleton() { return _suede_dptr()->GetSkeleton(); }
void Animation::SetRootTransform(const Matrix4& value) { _suede_dptr()->SetRootTransform(value); }
Matrix4 Animation::GetRootTransform() { return _suede_dptr()->GetRootTransform(); }
void Animation::SetWrapMode(AnimationWrapMode value) { _suede_dptr()->SetWrapMode(value); }
AnimationWrapMode Animation::GetWrapMode() { return _suede_dptr()->GetWrapMode(); }
bool Animation::Play(const std::string& name) { return _suede_dptr()->Play(name); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Animation, Component)

#define DEFAULT_TICKS_PER_SECOND	25

static void SetVariant(AnimationFrame* frame, int id, const Variant& value);
static void LerpVariant(Variant& variant, const Variant& lhs, const Variant& rhs, float factor);

bool SkeletonInternal::AddBone(const SkeletonBone& bone) {
	if (GetBone(bone.name) != nullptr) {
		return false;
	}

	if (current_ >= MAX_BONE_COUNT) {
		Debug::LogError("too many bones");
		return false;
	}

	bones_[current_] = bone;

	boneMap_.insert(std::make_pair(bone.name, current_));
	++current_;

	return true;
}

SkeletonBone* SkeletonInternal::GetBone(uint index) {
	SUEDE_ASSERT(index < current_);
	return bones_ + index;
}

SkeletonBone* SkeletonInternal::GetBone(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return nullptr;
	}

	return bones_ + pos->second;
}

void SkeletonInternal::SetBoneToRootMatrix(uint index, const Matrix4& value) {
	SUEDE_ASSERT(index < current_);
	boneToRootMatrices_[index] = value;
}

int SkeletonInternal::GetBoneIndex(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return -1;
	}

	return pos->second;
}

SkeletonNode* SkeletonInternal::CreateNode(const std::string& name, const Matrix4& matrix, AnimationCurve* curve) {
	SkeletonNode* node = new SkeletonNode;
	node->name = name;
	node->matrix = matrix;
	node->curve = curve;
	node->parent = nullptr;
	return node;
}

void SkeletonInternal::AddNode(SkeletonNode* parent, SkeletonNode* child) {
	if (parent == nullptr) {
		DestroyNodeHierarchy(root_);
		root_ = child;
	}
	else {
		child->parent = parent;
		parent->children.push_back(child);
	}
}

void SkeletonInternal::DestroyNodeHierarchy(SkeletonNode*& node) {
	if (node == nullptr) { return; }
	for (int i = 0; i < node->children.size(); ++i) {
		DestroyNodeHierarchy(node->children[i]);
		delete node->children[i];
	}

	delete node;
	node = nullptr;
}

AnimationClipInternal::AnimationClipInternal() : ObjectInternal(ObjectType::AnimationClip), wrapper_(Mathf::Min) {
	frame_ = new AnimationFrame();
}

AnimationClipInternal::~AnimationClipInternal() {
	delete frame_;
}

void AnimationClipInternal::SetWrapMode(AnimationWrapMode value) {
	switch (wrapMode_ = value) {
	case AnimationWrapMode::Loop:
		wrapper_ = Mathf::Repeat;
		break;
	case AnimationWrapMode::PingPong:
		wrapper_ = Mathf::PingPong;
		break;
	case AnimationWrapMode::Once:
	case AnimationWrapMode::ClampForever:
		wrapper_ = Mathf::Min;
		break;
	}
}

void AnimationClipInternal::SetTicksPerSecond(float value) {
	if (Mathf::Approximately(value, 0)) {
		value = DEFAULT_TICKS_PER_SECOND;
	}

	ticksInSecond_ = value;
}

bool AnimationClipInternal::Sample(float time) {
	time = wrapper_(time * GetTicksPerSecond(), GetDuration());

	SkeletonNode* root = GetAnimation()->GetSkeleton()->GetRootNode();
	return SampleHierarchy(time, root, Matrix4(1));
}

bool AnimationClipInternal::SampleHierarchy(float time, SkeletonNode* node, const Matrix4& matrix) {
	bool endFrame = true;
	Matrix4 transform = node->matrix;
	if (node->curve) {
		endFrame = node->curve->Sample(time, frame_);

		Quaternion rotation = frame_->GetQuaternion(FrameKeyRotation);
		Vector3 position = frame_->GetVector3(FrameKeyPosition);
		Vector3 scale = frame_->GetVector3(FrameKeyScale);

		transform = Matrix4::TRS(position, rotation, scale);
	}

	transform = matrix * transform;

	Skeleton* skeleton = GetAnimation()->GetSkeleton();
	int index = skeleton->GetBoneIndex(node->name);
	if (index >= 0) {
		Matrix4 boneToRootMatrix = GetAnimation()->GetRootTransform();
		boneToRootMatrix *= transform * skeleton->GetBone(index)->meshToBoneMatrix;

		skeleton->SetBoneToRootMatrix(index, boneToRootMatrix);
	}

	for (int i = 0; i < node->children.size(); ++i) {
		endFrame = SampleHierarchy(time, node->children[i], transform) && endFrame;
	}

	return endFrame;
}

AnimationKeysInternal::AnimationKeysInternal() :ObjectInternal(ObjectType::AnimationKeys) {
	container_.resize(FrameKeyMaxCount);
}

AnimationKeysInternal::~AnimationKeysInternal() {
	for (KeysContainer::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
		delete *ite;
	}
}

void AnimationKeysInternal::AddFloat(float time, int id, float value) {
	InsertKey(time, Key{ id, value });
}

void AnimationKeysInternal::AddVector3(float time, int id, const Vector3& value) {
	InsertKey(time, Key{ id, value });
}

void AnimationKeysInternal::AddQuaternion(float time, int id, const Quaternion& value) {
	InsertKey(time, Key{ id, value });
}

void AnimationKeysInternal::Remove(float time, int id) {
	RemoveKey(id, time);
}

void AnimationKeysInternal::ToKeyframes(std::vector<ref_ptr<AnimationFrame>>& keyframes) {
	int count = SmoothKeys();
	if (count != 0) {
		InitializeKeyframes(count, keyframes);
	}
}

void AnimationKeysInternal::InitializeKeyframes(int count, std::vector<ref_ptr<AnimationFrame>>& keyframes) {
	keyframes.reserve(count);

	for (int i = 0; i < count; ++i) {
		AnimationFrame* keyframe = nullptr;

		for (KeysContainer::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
			if (*ite == nullptr) { continue; }

			Keys::iterator p = (*ite)->begin();
			std::advance(p, i);

			const Key& key = p->second;

			if (keyframe == nullptr) {
				keyframe = new AnimationFrame();
				keyframe->SetTime(p->first);
				keyframes.push_back(keyframe);
			}

			SetVariant(keyframe, key.id, key.value);
		}
	}
}

int AnimationKeysInternal::SmoothKeys() {
	std::set<float> times;
	for (KeysContainer::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
		if (*ite == nullptr) { continue; }
		Keys* keys = (*ite);
		for (Keys::iterator ite2 = keys->begin(); ite2 != keys->end(); ++ite2) {
			times.insert(ite2->first);
		}
	}

	for (std::set<float>::iterator ite = times.begin(); ite != times.end(); ++ite) {
		float time = *ite;
		for (KeysContainer::iterator ite2 = container_.begin(); ite2 != container_.end(); ++ite2) {
			if (*ite2 == nullptr) { continue; }
			SmoothKey(*ite2, time);
		}
	}

	return times.size();
}

void AnimationKeysInternal::InsertKey(float time, const Key& key) {
	if (key.id >= FrameKeyMaxCount) {
		Debug::LogError("id must be less than %d.",FrameKeyMaxCount);
		return;
	}

	Keys* keys = container_[key.id];

	if (container_[key.id] == nullptr) {
		container_[key.id] = keys = new Keys;
	}

	keys->insert(std::make_pair(time, key));
}

void AnimationKeysInternal::RemoveKey(int id, float time) {
	Keys* keys = container_[id];
	if (keys == nullptr) {
		return;
	}

	keys->erase(time);
	if (keys->empty()) {
		delete keys;
		container_[id] = nullptr;
	}
}

void AnimationKeysInternal::SmoothKey(Keys* keys, float time) {
	if (keys->empty()) {
		Debug::LogError("empty key container");
		return;
	}

	Keys::iterator pos = keys->find(time);
	if (pos != keys->end()) {
		return;
	}

	Key key;
	key.id = keys->begin()->second.id;

	if (pos == keys->end()) {
		key.value = (--keys->end())->second.value;
	}
	else {
		if (pos == keys->begin()) {
			key.value = keys->begin()->second.value;
		}
		else {
			Keys::iterator prev = pos;
			--prev;

			float t = Mathf::Clamp01((time - prev->first) / (pos->first - prev->first));
			LerpVariant(key.value, prev->second.value, pos->second.value, t);
		}
	}

	keys->insert(std::make_pair(time, key));
}

void AnimationInternal::AddClip(Animation* self, const std::string& name, AnimationClip* value) {
	clips_.insert(std::make_pair(name, value));
	value->SetAnimation(self);
}

AnimationClip* AnimationInternal::GetClip(const std::string& name) {
	ClipContainer::iterator pos = clips_.find(name);
	if (pos == clips_.end()) {
		return nullptr;
	}

	return pos->second.get();
}

void AnimationInternal::SetWrapMode(AnimationWrapMode value) {
	wrapMode_ = value;
	for (ClipContainer::iterator ite = clips_.begin(); ite != clips_.end(); ++ite) {
		ite->second->SetWrapMode(value);
	}
}

bool AnimationInternal::Play(const std::string& name) {
	AnimationClip* clip = GetClip(name);
	if (!clip) {
		Debug::LogWarning("can not find animation clip %s.", name.c_str());
		return false;
	}

	time_ = 0;
	current_ = clip;
	playing_ = true;

	return true;
}

void AnimationInternal::CullingUpdate(float deltaTime) {
	if (playing_ && current_) {
		time_ += deltaTime;

		if (current_->Sample(time_) && current_->GetWrapMode() == +AnimationWrapMode::Once) {
			current_->Sample(0);
			playing_ = false;
		}
	}
}

void AnimationCurveInternal::SetKeys(AnimationKeys* value) {
	keyframes_.clear();
	value->ToKeyframes(keyframes_);
}

bool AnimationCurveInternal::Sample(float time, AnimationFrame*& frame) {
	int index = FindInterpolateIndex(time);
	if (index + 1 >= keyframes_.size()) {
		SampleEndFrame(frame);
		return true;
	}

	Lerp(index, time, frame);
	return false;
}

int AnimationCurveInternal::FindInterpolateIndex(float time) {
	struct Comparerer {
		bool operator ()(ref_ptr<AnimationFrame>& lhs, float time) const {
			return lhs->GetTime() < time;
		}
	};

	return (int)std::distance(std::lower_bound(keyframes_.begin(), keyframes_.end(), time, Comparerer()), keyframes_.end());
}

void AnimationCurveInternal::SampleEndFrame(AnimationFrame*& frame) {
	if (!keyframes_.empty()) {
		frame->Assign(keyframes_.back().get());
	}
}

void AnimationCurveInternal::Lerp(int index, float time, AnimationFrame*& frame) {
	int next = index + 1;

	float deltaTime = keyframes_[next]->GetTime() - keyframes_[index]->GetTime();
	float factor = (time - keyframes_[index]->GetTime()) / deltaTime;

	factor = Mathf::Clamp01(factor);
	keyframes_[index]->Lerp(frame, keyframes_[next].get(), factor);
}

void AnimationFrameInternal::Lerp(AnimationFrame* result, AnimationFrame* other, float factor) {
	AttributeContainer& otherAttributes = _suede_drptr(other)->attributes_;
	if (attributes_.size() != otherAttributes.size()) {
		Debug::LogError("attribute count mismatch");
		return;
	}

	for (AttributeContainer::iterator ite = attributes_.begin(), ite2 = otherAttributes.begin(); ite != attributes_.end(); ++ite, ++ite2) {
		int id = ite->first, id2 = ite2->first;
		Variant& variant = ite->second, &variant2 = ite2->second;
		if (id != id2) {
			Debug::LogError("attribute id mismatch");
			continue;
		}

		if (variant.GetType() != variant2.GetType()) {
			Debug::LogError("attribute type mismatch");
			continue;
		}

		result->SetTime(Mathf::Lerp(time_, other->GetTime(), factor));
		LerpAttribute(result, id, variant, variant2, factor);
	}
}

void AnimationFrameInternal::Assign(AnimationFrame* other) {
 	time_ = _suede_drptr(other)->time_;
 	attributes_ = _suede_drptr(other)->attributes_;
}

void AnimationFrameInternal::LerpAttribute(AnimationFrame* ans, int id, const Variant& lhs, const Variant& rhs, float factor) {
	VariantType type = lhs.GetType();
	Variant variant;
	LerpVariant(variant, lhs, rhs, factor);
	SetVariant(ans, id, variant);
}

float AnimationFrameInternal::GetFloat(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if(pos == attributes_.end()) {
		Debug::LogError("keyframe attribute for id %d does not exist.", id);
		return 0;
	}

	return pos->second.GetFloat();
}


Vector3 AnimationFrameInternal::GetVector3(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if (pos == attributes_.end()) {
		Debug::LogError("keyframe attribute for id %d does not exist.", id);
		return Vector3(0);
	}

	return pos->second.GetVector3();
}

Quaternion AnimationFrameInternal::GetQuaternion(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if (pos == attributes_.end()) {
		Debug::LogError("keyframe attribute for id %d does not exist.", id);
		return Quaternion();
	}

	return pos->second.GetQuaternion();
}

static void SetVariant(AnimationFrame* frame, int id, const Variant& value) {
	VariantType type = value.GetType();
	switch (type) {
		case VariantType::Float:
			frame->SetFloat(id, value.GetFloat());
			break;
		case VariantType::Vector3:
			frame->SetVector3(id, value.GetVector3());
			break;
		case VariantType::Quaternion:
			frame->SetQuaternion(id, value.GetQuaternion());
			break;
		default:
			Debug::LogError("can not set variant %d.", type);
			break;
	}
}

static void LerpVariant(Variant& variant, const Variant& lhs, const Variant& rhs, float factor) {
	if (lhs.GetType() != rhs.GetType()) {
		Debug::LogError("variant type mismatch");
		return;
	}

	VariantType type = lhs.GetType();

	switch (type) {
		case VariantType::Float:
			variant.SetFloat(Mathf::Lerp(lhs.GetFloat(), rhs.GetFloat(), factor));
			break;
		case VariantType::Vector3:
			variant.SetVector3(Vector3::Lerp(lhs.GetVector3(), rhs.GetVector3(), factor));
			break;
		case VariantType::Quaternion:
			variant.SetQuaternion(Quaternion::Lerp(lhs.GetQuaternion(), rhs.GetQuaternion(), factor));
			break;
		default:
			Debug::LogError("can not lerp attribute type %d.", type);
			break;
	}
}
