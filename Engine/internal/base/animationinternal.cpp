#include <set>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "math2.h"
#include "time2.h"
#include "debug.h"
#include "animationinternal.h"

#define DEFAULT_TICKS_PER_SECOND	25

static void SetVariant(AnimationFrame frame, int id, Variant& value);
static void LerpVariant(Variant& variant, Variant& lhs, Variant& rhs, float factor);

bool SkeletonInternal::AddBone(const SkeletonBone& bone) {
	if (GetBone(bone.name) != nullptr) {
		return false;
	}

	if (current_ >= C_MAX_BONE_COUNT) {
		Debug::LogError("too many bones");
		return false;
	}

	bones_[current_] = bone;

	boneMap_.insert(std::make_pair(bone.name, current_));
	++current_;

	return true;
}

SkeletonBone* SkeletonInternal::GetBone(uint index) {
	if (index >= current_) {
		Debug::LogError("index out of range");
		return nullptr;
	}

	return bones_ + index;
}

SkeletonBone* SkeletonInternal::GetBone(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return nullptr;
	}

	return bones_ + pos->second;
}

void SkeletonInternal::SetBoneToRootSpaceMatrix(uint index, const glm::mat4& value) {
	if (index >= current_) {
		Debug::LogError("index out of range");
		return;
	}

	boneToRootSpaceMatrices_[index] = value;
}

int SkeletonInternal::GetBoneIndex(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return -1;
	}

	return pos->second;
}

SkeletonNode* SkeletonInternal::CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve) {
	SkeletonNode* node = MEMORY_CREATE(SkeletonNode);
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
		MEMORY_RELEASE(node->children[i]);
	}

	MEMORY_RELEASE(node);
	node = nullptr;
}

AnimationClipInternal::AnimationClipInternal() : ObjectInternal(ObjectTypeAnimationClip), wrapper_(Math::Min) {
	frame_ = CREATE_OBJECT(AnimationFrame);
}

void AnimationClipInternal::SetWrapMode(AnimationWrapMode value) {
	switch (wrapMode_ = value) {
	case AnimationWrapModeLoop:
		wrapper_ = Math::Repeat;
		break;
	case AnimationWrapModePingPong:
		wrapper_ = Math::PingPong;
		break;
	case AnimationWrapModeOnce:
	case AnimationWrapModeClampForever:
		wrapper_ = Math::Min;
		break;
	}
}

void AnimationClipInternal::SetTicksPerSecond(float value) {
	if (Math::Approximately(value)) {
		value = DEFAULT_TICKS_PER_SECOND;
	}

	ticksInSecond_ = value;
}

bool AnimationClipInternal::Sample(float time) {
	time = wrapper_(time * GetTicksPerSecond(), GetDuration());

	Skeleton skeleton = GetAnimation()->GetSkeleton();
	SkeletonNode* root = skeleton->GetRootNode();

	return SampleHierarchy(time, root, glm::mat4(1));
}

bool AnimationClipInternal::SampleHierarchy(float time, SkeletonNode* node, const glm::mat4& matrix) {
	bool lastFrame = true;
	glm::mat4 transform = node->matrix;
	if (node->curve) {
		lastFrame = node->curve->Sample(time, frame_);

		glm::quat rotation = frame_->GetQuaternion(FrameKeyRotation);
		glm::vec3 position = frame_->GetVector3(FrameKeyPosition);
		glm::vec3 scale = frame_->GetVector3(FrameKeyScale);

		glm::mat4 identity;
		transform = glm::translate(identity, position) * glm::mat4(rotation) * glm::scale(identity, scale);
	}

	transform = matrix * transform;

	Skeleton skeleton = GetAnimation()->GetSkeleton();
	int index = skeleton->GetBoneIndex(node->name);
	if (index >= 0) {
		glm::mat4 boneToRootSpaceMatrix = GetAnimation()->GetRootTransform();
		boneToRootSpaceMatrix *= transform * skeleton->GetBone(index)->localToBoneSpaceMatrix;

		skeleton->SetBoneToRootSpaceMatrix(index, boneToRootSpaceMatrix);
	}

	for (int i = 0; i < node->children.size(); ++i) {
		lastFrame = SampleHierarchy(time, node->children[i], transform) && lastFrame;
	}

	return lastFrame;
}

AnimationKeysInternal::AnimationKeysInternal() :ObjectInternal(ObjectTypeAnimationKeys) {
	container_.resize(FrameKeyMaxCount);
}

AnimationKeysInternal::~AnimationKeysInternal() {
	for (Container::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
		MEMORY_RELEASE(*ite);
	}
}

void AnimationKeysInternal::AddFloat(int id, float time, float value) {
	Key key{ id, time };
	key.value.SetFloat(value);
	InsertKey(id, key);
}

void AnimationKeysInternal::AddVector3(int id, float time, const glm::vec3& value) {
	Key key{ id, time };
	key.value.SetVector3(value);
	InsertKey(id, key);
}

void AnimationKeysInternal::AddQuaternion(int id, float time, const glm::quat& value) {
	Key key{ id, time };
	key.value.SetQuaternion(value);
	InsertKey(id, key);
}

void AnimationKeysInternal::Remove(int id, float time) {
	Key key{ id, time };
	RemoveKey(key);
}

void AnimationKeysInternal::ToKeyframes(std::vector<AnimationFrame>& keyframes) {
	int count = SmoothKeys();
	if (count != 0) {
		InitializeKeyframes(count, keyframes);
	}
}

void AnimationKeysInternal::InitializeKeyframes(int count, std::vector<AnimationFrame> &keyframes) {
	keyframes.reserve(count);

	for (int i = 0; i < count; ++i) {
		AnimationFrame keyframe;

		for (Container::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
			if (*ite == nullptr) { continue; }

			Key& key = (*ite)->at(i);

			if (!keyframe) {
				keyframe = CREATE_OBJECT(AnimationFrame);
				keyframe->SetTime(key.time);
				keyframes.push_back(keyframe);
			}

			SetVariant(keyframe, key.id, key.value);
		}
	}
}

int AnimationKeysInternal::SmoothKeys() {
	typedef std::set<float> TimeLine;
	TimeLine times;
	for (Container::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
		if (*ite == nullptr) { continue; }
		Keys* keys = (*ite);
		for (Keys::iterator ite2 = keys->begin(); ite2 != keys->end(); ++ite2) {
			times.insert(ite2->time);
		}
	}

	for (TimeLine::iterator ite = times.begin(); ite != times.end(); ++ite) {
		float time = *ite;
		for (Container::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
			if (*ite == nullptr) { continue; }
			SmoothKey(*ite, time);
		}
	}

	return times.size();
}

void AnimationKeysInternal::InsertKey(uint id, const Key& key) {
	if (id >= FrameKeyMaxCount) {
		Debug::LogError("id must be less than %d.",FrameKeyMaxCount);
		return;
	}

	Keys* keys = container_[id];
	if (container_[id] == nullptr) {
		container_[id] = keys = MEMORY_CREATE(Keys);
	}

	keys->insert(key);
}

void AnimationKeysInternal::RemoveKey(const Key& key) {
	Keys* keys = container_[key.id];
	if (keys == nullptr) {
		return;
	}

	keys->remove(key);
	if (keys->empty()) {
		MEMORY_RELEASE(keys);
		container_[key.id] = nullptr;
	}
}

void AnimationKeysInternal::SmoothKey(Keys* keys, float time) {
	if (keys->empty()) {
		Debug::LogError("empty key container");
		return;
	}

	Keys::value_type key;
	key.time = time;

	Keys::iterator pos = keys->find(key);
	if (pos != keys->end() && Math::Approximately(pos->time, time)) {
		return;
	}

	key.id = keys->front().id;

	if (pos == keys->end()) {
		key.value = keys->back().value;
	}
	else {
		if (pos == keys->begin()) {
			key.value = keys->front().value;
		}
		else {
			Keys::iterator prev = pos;
			--prev;

			float t = Math::Clamp01((time - prev->time) / (pos->time - prev->time));
			LerpVariant(key.value, prev->value, pos->value, t);
		}
	}

	keys->insert(key);
}

void AnimationInternal::AddClip(const std::string& name, AnimationClip value) {
	Key key{ name, value };
	clips_.insert(key);

	value->SetAnimation(dsp_cast<Animation>(shared_from_this()));
}

AnimationClip AnimationInternal::GetClip(const std::string& name) {
	Key key{ name };
	if (!clips_.get(key)) { return nullptr; }

	return key.value;
}

void AnimationInternal::SetWrapMode(AnimationWrapMode value) {
	for (int i = 0; i < clips_.size(); ++i) {
		clips_[i].value->SetWrapMode(value);
	}
}

bool AnimationInternal::Play(const std::string& name) {
	AnimationClip clip = GetClip(name);
	if (!clip) {
		Debug::LogWarning("can not find animation clip %s.", name.c_str());
		return false;
	}

	time_ = 0;
	current_ = clip;
	playing_ = true;

	return true;
}

void AnimationInternal::Update() {
	if (!playing_ || !current_) { return; }

	time_ += Time::GetDeltaTime();
	
	if (current_->Sample(time_) && current_->GetWrapMode() == AnimationWrapModeOnce) {
		current_->Sample(0);
		playing_ = false;
	}
}

bool AnimationCurveInternal::Sample(float time, AnimationFrame& frame) {
	int index = FindInterpolateIndex(time);
	if (index + 1 >= keyframes_.size()) {
		SampleLastFrame(frame);
		return true;
	}

	Lerp(index, time, frame);
	return false;
}

int AnimationCurveInternal::FindInterpolateIndex(float time) {
	struct Comparerer {
		bool operator ()(AnimationFrame& lhs, float time) const {
			return lhs->GetTime() < time;
		}
	};

	return (int)std::distance(std::lower_bound(keyframes_.begin(), keyframes_.end(), time, Comparerer()), keyframes_.end());
}

void AnimationCurveInternal::SampleLastFrame(AnimationFrame& frame) {
	if (!keyframes_.empty()) {
		frame->Assign(keyframes_.back());
	}
}

void AnimationCurveInternal::Lerp(int index, float time, AnimationFrame& frame) {
	int next = index + 1;

	float deltaTime = keyframes_[next]->GetTime() - keyframes_[index]->GetTime();
	float factor = (time - keyframes_[index]->GetTime()) / deltaTime;

	factor = Math::Clamp01(factor);
	keyframes_[index]->Lerp(frame, keyframes_[next], factor);
}

void AnimationFrameInternal::Lerp(AnimationFrame result, AnimationFrame other, float factor) {
	SortedVector<Key>& otherAttributes = ((AnimationFrameInternal*)(other.get()))->attributes_;
	if (attributes_.size() != otherAttributes.size()) {
		Debug::LogError("attribute count mismatch");
		return;
	}

	for (int i = 0; i < attributes_.size(); ++i) {
		Key& lhs = attributes_[i], &rhs = otherAttributes[i];
		if (lhs.id != rhs.id) {
			Debug::LogError("attribute id mismatch");
			continue;
		}

		if (lhs.value.GetType() != rhs.value.GetType()) {
			Debug::LogError("attribute type mismatch");
			continue;
		}

		result->SetTime(Math::Lerp(time_, other->GetTime(), factor));
		LerpAttribute(result, lhs, rhs, factor);
	}
}

void AnimationFrameInternal::Assign(AnimationFrame other) {
	AnimationFrameInternal* ptr = ((AnimationFrameInternal*)(other.get()));
	time_ = ptr->time_;
	attributes_ = ptr->attributes_;
}

void AnimationFrameInternal::LerpAttribute(AnimationFrame ans, Key& lhs, Key& rhs, float factor) {
	VariantType type = lhs.value.GetType();
	Variant variant;
	LerpVariant(variant, lhs.value, rhs.value, factor);
	SetVariant(ans, lhs.id, variant);
}

void AnimationFrameInternal::SetFloat(int id, float value) {
	Key key = { id };
	key.value.SetFloat(value);
	attributes_.insert(key);
}

void AnimationFrameInternal::SetVector3(int id, const glm::vec3& value) {
	Key key = { id };
	key.value.SetVector3(value);
	attributes_.insert(key);
}

void AnimationFrameInternal::SetQuaternion(int id, const glm::quat& value) {
	Key key = { id };
	key.value.SetQuaternion(value);
	attributes_.insert(key);
}

float AnimationFrameInternal::GetFloat(int id) {
	Key key{ id };
	if (!attributes_.get(key)) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return 0;
	}

	return key.value.GetFloat();
}


glm::vec3 AnimationFrameInternal::GetVector3(int id) {
	Key key{ id };
	if (!attributes_.get(key)) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return glm::vec3(0);
	}

	return key.value.GetVector3();
}

glm::quat AnimationFrameInternal::GetQuaternion(int id) {
	Key key{ id };
	if (!attributes_.get(key)) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return glm::quat();
	}

	return key.value.GetQuaternion();
}

static void SetVariant(AnimationFrame frame, int id, Variant& value) {
	VariantType type = value.GetType();
	switch (type) {
		case VariantTypeFloat:
			frame->SetFloat(id, value.GetFloat());
			break;
		case VariantTypeVector3:
			frame->SetVector3(id, value.GetVector3());
			break;
		case VariantTypeQuaternion:
			frame->SetQuaternion(id, value.GetQuaternion());
			break;
		default:
			Debug::LogError("can not set variant %d.", type);
			break;
	}
}

static void LerpVariant(Variant& variant, Variant& lhs, Variant& rhs, float factor) {
	if (lhs.GetType() != rhs.GetType()) {
		Debug::LogError("variant type mismatch");
		return;
	}

	VariantType type = lhs.GetType();

	switch (type) {
		case VariantTypeFloat:
			variant.SetFloat(Math::Lerp(lhs.GetFloat(), rhs.GetFloat(), factor));
			break;
		case VariantTypeVector3:
			variant.SetVector3(Math::Lerp(lhs.GetVector3(), rhs.GetVector3(), factor));
			break;
		case VariantTypeQuaternion:
			variant.SetQuaternion(Math::Lerp(lhs.GetQuaternion(), rhs.GetQuaternion(), factor));
			break;
		default:
			Debug::LogError("can not lerp attribute type %d.", type);
			break;
	}
}
