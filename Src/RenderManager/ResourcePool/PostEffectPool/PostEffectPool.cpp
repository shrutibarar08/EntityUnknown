#include "PostEffectPool.h"
#include "RenderManager/PostEffect/PostEffect.h"

#include "Utils/Logger/Logger.h"
#include "Utils/HelperFunctions.h"

ID PostEffectPool::Add(const EU_POST_EFFECT_INIT_DESC& desc)
{
	if (ID id = IsExits(desc); id)
	{
		return id;
	}

	PostEffectData data{};
	data.BlobDesc	= desc.BlobDesc;
	data.EffectName = desc.EffectName;
	data.pEffect	= std::make_unique<PostEffect>(desc);
	ID createdId	= data.pEffect->GetAssignedID();

	m_mapCachedEffectsPool[createdId] = std::move(data);
	m_bDirty = true;

	LOG_WARNING("Created " + desc.EffectName + " On ID: " + std::to_string(createdId));

	return createdId;
}

bool PostEffectPool::Remove(ID id)
{
	if (!m_mapCachedEffectsPool.contains(id)) return false;
	m_mapCachedEffectsPool.erase(id);
	return true;
}

bool PostEffectPool::Rename(ID id, const std::string& newName)
{
	if (!m_mapCachedEffectsPool.contains(id)) return false;
	m_mapCachedEffectsPool[id].EffectName = newName;
	return true;
}

EU_POST_EFFECT_SHARED_VIEW PostEffectPool::GetEffectByID(ID id) const
{
	if (m_mapCachedEffectsPool.contains(id))
	{
		EU_POST_EFFECT_SHARED_VIEW view{};
		view.EffectID = id;
		view.EffectName = m_mapCachedEffectsPool.at(id).EffectName;
		view.pEffect = m_mapCachedEffectsPool.at(id).pEffect.get();
		view.BlobDesc = m_mapCachedEffectsPool.at(id).BlobDesc;
		return view;
	}
	return {};
}

const std::vector<EU_POST_EFFECT_SHARED_VIEW>& PostEffectPool::GetAllEffects()
{
	if (m_bDirty)
	{
		m_ppViewEffectsPool.clear();

		for (auto& [id, data] : m_mapCachedEffectsPool)
		{
			EU_POST_EFFECT_SHARED_VIEW view{};
			view.EffectID	= id;
			view.EffectName = data.EffectName;
			view.pEffect	= data.pEffect.get();
			view.BlobDesc   = data.BlobDesc;
			m_ppViewEffectsPool.emplace_back(view);
		}
		m_bDirty = false;
	}
	return m_ppViewEffectsPool;
}

ID PostEffectPool::IsExits(const EU_POST_EFFECT_INIT_DESC& desc)
{
	for (const auto& [id, effect] : m_mapCachedEffectsPool)
	{
		if (effect.EffectName == desc.EffectName)
		{
			if (IsBlobDescSame(id, effect.BlobDesc))
			{
				return effect.pEffect->GetAssignedID(); // cached
			}
			else return 0u; // path same but blob not same
		}
	}
	return 0u; // not cached at all
}

bool PostEffectPool::IsBlobDescSame(ID id, const EU_BLOB_INIT_DESC& right) const
{
	if (!m_mapCachedEffectsPool.contains(id)) return false;
	const auto& ourBlob = m_mapCachedEffectsPool.at(id).BlobDesc;
	return ourBlob == right;
}
