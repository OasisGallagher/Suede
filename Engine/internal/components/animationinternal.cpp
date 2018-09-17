#include "animationinternal.h"

#include <set>
#include <algorithm>

#include "time2.h"
#include "tools/math2.h"

SUEDE_DEFINE_COMPONENT(IAnimation, IComponent)

#define DEFAULT_TICKS_PER_SECOND	25

static void SetVariant(AnimationFrame frame, int id, const Variant& value);
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
	VERIFY_INDEX(index, current_, nullptr);
	return bones_ + index;
}

SkeletonBone* SkeletonInternal::GetBone(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return nullptr;
	}

	return bones_ + pos->second;
}

void SkeletonInternal::SetBoneToRootMatrix(uint index, const glm::mat4& value) {
	VERIFY_INDEX(index, current_, NOARG);
	boneToRootMatrices_[index] = value;
}

int SkeletonInternal::GetBoneIndex(const std::string& name) {
	std::map<std::string, int>::iterator pos = boneMap_.find(name);
	if (pos == boneMap_.end()) {
		return -1;
	}

	return pos->second;
}

SkeletonNode* SkeletonInternal::CreateNode(const std::string& name, const glm::mat4& matrix, AnimationCurve curve) {
	SkeletonNode* node = MEMORY_NEW(SkeletonNode);
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
		MEMORY_DELETE(node->children[i]);
	}

	MEMORY_DELETE(node);
	node = nullptr;
}

AnimationClipInternal::AnimationClipInternal() : ObjectInternal(ObjectType::AnimationClip), wrapper_(Math::Min) {
	frame_ = NewAnimationFrame();
}

void AnimationClipInternal::SetWrapMode(AnimationWrapMode value) {
	switch (wrapMode_ = value) {
	case AnimationWrapMode::Loop:
		wrapper_ = Math::Repeat;
		break;
	case AnimationWrapMode::PingPong:
		wrapper_ = Math::PingPong;
		break;
	case AnimationWrapMode::Once:
	case AnimationWrapMode::ClampForever:
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
	bool endFrame = true;
	glm::mat4 transform = node->matrix;
	if (node->curve) {
		endFrame = node->curve->Sample(time, frame_);

		glm::quat rotation = frame_->GetQuaternion(FrameKeyRotation);
		glm::vec3 position = frame_->GetVector3(FrameKeyPosition);
		glm::vec3 scale = frame_->GetVector3(FrameKeyScale);

		transform = Math::TRS(position, rotation, scale);
	}

	transform = matrix * transform;

	Skeleton skeleton = GetAnimation()->GetSkeleton();
	int index = skeleton->GetBoneIndex(node->name);
	if (index >= 0) {
		glm::mat4 boneToRootMatrix = GetAnimation()->GetRootTransform();
		boneToRootMatrix *= transform * skeleton->GetBone(index)->localToBoneMatrix;

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
		MEMORY_DELETE(*ite);
	}
}

void AnimationKeysInternal::AddFloat(float time, int id, float value) {
	Key key{ id };
	key.value.SetFloat(value);
	InsertKey(time, key);
}

void AnimationKeysInternal::AddVector3(float time, int id, const glm::vec3& value) {
	Key key{ id };
	key.value.SetVector3(value);
	InsertKey(time, key);
}

void AnimationKeysInternal::AddQuaternion(float time, int id, const glm::quat& value) {
	Key key{ id };
	key.value.SetQuaternion(value);
	InsertKey(time, key);
}

void AnimationKeysInternal::Remove(float time, int id) {
	RemoveKey(id, time);
}

void AnimationKeysInternal::ToKeyframes(std::vector<AnimationFrame>& keyframes) {
	int count = SmoothKeys();
	if (count != 0) {
		InitializeKeyframes(count, keyframes);
	}
}

void AnimationKeysInternal::InitializeKeyframes(int count, std::vector<AnimationFrame>& keyframes) {
	keyframes.reserve(count);

	for (int i = 0; i < count; ++i) {
		AnimationFrame keyframe;

		for (KeysContainer::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
			if (*ite == nullptr) { continue; }

			Keys::iterator p = (*ite)->begin();
			std::advance(p, i);

			const Key& key = p->second;

			if (!keyframe) {
				keyframe = NewAnimationFrame();
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
		for (KeysContainer::iterator ite = container_.begin(); ite != container_.end(); ++ite) {
			if (*ite == nullptr) { continue; }
			SmoothKey(*ite, time);
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
		container_[key.id] = keys = MEMORY_NEW(Keys);
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
		MEMORY_DELETE(keys);
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

			float t = Math::Clamp01((time - prev->first) / (pos->first - prev->first));
			LerpVariant(key.value, prev->second.value, pos->second.value, t);
		}
	}

	keys->insert(std::make_pair(time, key));
}

bool AnimationKeysInternal::FloatCamparer::operator()(float lhs, float rhs) const {
	return !Math::Approximately(lhs, rhs) && lhs < rhs;
}

void AnimationInternal::AddClip(const std::string& name, AnimationClip value) {
	clips_.insert(std::make_pair(name, value));
	value->SetAnimation(SharedThis());
}

AnimationClip AnimationInternal::GetClip(const std::string& name) {
	ClipContainer::iterator pos = clips_.find(name);
	if (pos == clips_.end()) {
		return nullptr;
	}

	return pos->second;
}

void AnimationInternal::SetWrapMode(AnimationWrapMode value) {
	for (ClipContainer::iterator ite = clips_.begin(); ite != clips_.end(); ++ite) {
		ite->second->SetWrapMode(value);
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

void AnimationInternal::CullingUpdate() {
	if (!playing_ || !current_) { return; }

	time_ += Time::instance()->GetDeltaTime();

	if (current_->Sample(time_) && current_->GetWrapMode() == +AnimationWrapMode::Once) {
		current_->Sample(0);
		playing_ = false;
	}
}

void AnimationInternal::RenderingUpdate() {

}

bool AnimationCurveInternal::Sample(float time, AnimationFrame& frame) {
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
		bool operator ()(AnimationFrame& lhs, float time) const {
			return lhs->GetTime() < time;
		}
	};

	return (int)std::distance(std::lower_bound(keyframes_.begin(), keyframes_.end(), time, Comparerer()), keyframes_.end());
}

void AnimationCurveInternal::SampleEndFrame(AnimationFrame& frame) {
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
	AttributeContainer& otherAttributes = ((AnimationFrameInternal*)(other.get()))->attributes_;
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

		result->SetTime(Math::Lerp(time_, other->GetTime(), factor));
		LerpAttribute(result, id, variant, variant2, factor);
	}
}

void AnimationFrameInternal::Assign(AnimationFrame other) {
	AnimationFrameInternal* ptr = ((AnimationFrameInternal*)(other.get()));
	time_ = ptr->time_;
	attributes_ = ptr->attributes_;
}

void AnimationFrameInternal::LerpAttribute(AnimationFrame ans, int id, const Variant& lhs, const Variant& rhs, float factor) {
	VariantType type = lhs.GetType();
	Variant variant;
	LerpVariant(variant, lhs, rhs, factor);
	SetVariant(ans, id, variant);
}

void AnimationFrameInternal::SetFloat(int id, float value) {
	Variant variant;
	variant.SetFloat(value);
	attributes_[id] = variant;
}

void AnimationFrameInternal::SetVector3(int id, const glm::vec3& value) {
	Variant variant;
	variant.SetVector3(value);
	attributes_[id] = variant;
}

void AnimationFrameInternal::SetQuaternion(int id, const glm::quat& value) {
	Variant variant;
	variant.SetQuaternion(value);
	attributes_[id] = variant;
}

float AnimationFrameInternal::GetFloat(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if(pos == attributes_.end()) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return 0;
	}

	return pos->second.GetFloat();
}


glm::vec3 AnimationFrameInternal::GetVector3(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if (pos == attributes_.end()) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return glm::vec3(0);
	}

	return pos->second.GetVector3();
}

glm::quat AnimationFrameInternal::GetQuaternion(int id) {
	AttributeContainer::iterator pos = attributes_.find(id);
	if (pos == attributes_.end()) {
		Debug::LogError("Animation keyframe attribute for id %d does not exist.", id);
		return glm::quat();
	}

	return pos->second.GetQuaternion();
}

static void SetVariant(AnimationFrame frame, int id, const Variant& value) {
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
			variant.SetFloat(Math::Lerp(lhs.GetFloat(), rhs.GetFloat(), factor));
			break;
		case VariantType::Vector3:
			variant.SetVector3(Math::Lerp(lhs.GetVector3(), rhs.GetVector3(), factor));
			break;
		case VariantType::Quaternion:
			variant.SetQuaternion(Math::Lerp(lhs.GetQuaternion(), rhs.GetQuaternion(), factor));
			break;
		default:
			Debug::LogError("can not lerp attribute type %d.", type);
			break;
	}
}
