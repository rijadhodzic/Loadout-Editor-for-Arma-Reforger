// Required companion class — every ScriptComponent must have one.
[ComponentEditorProps(category: "LoadoutEditor", description: "Server-side loadout editor manager. Attach to game mode entity.")]
class SCR_LoadoutEditorManagerClass : ScriptComponentClass
{
}

// Server-authority component — attach to the game mode entity in Workbench.
// Validates items against the arsenal's storage and applies the loadout.
class SCR_LoadoutEditorManager : ScriptComponent
{
	protected static SCR_LoadoutEditorManager s_Instance;

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutEditorManager GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gm = GetGame().GetGameMode();
			if (gm)
				s_Instance = SCR_LoadoutEditorManager.Cast(gm.FindComponent(SCR_LoadoutEditorManager));
		}
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	// Called from the client UI — triggers the server RPC from within this component's context.
	void RequestApplyLoadout(int playerID, string presetData, RplId arsenalRplId)
	{
		Rpc(Svr_ApplyLoadout, playerID, presetData, arsenalRplId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Svr_ApplyLoadout(int playerID, string presetData, RplId arsenalRplId)
	{
		// Resolve arsenal storage component
		RplComponent arsenalRpl = RplComponent.Cast(Replication.FindItem(arsenalRplId));
		if (!arsenalRpl)
		{
			Print("[LoadoutEditor] Svr_ApplyLoadout — RplId not found.", LogLevel.WARNING);
			return;
		}

		SCR_ArsenalInventoryStorageManagerComponent arsenalStorage =
			SCR_ArsenalInventoryStorageManagerComponent.Cast(
				arsenalRpl.GetEntity().FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		if (!arsenalStorage)
		{
			Print("[LoadoutEditor] Svr_ApplyLoadout — no SCR_ArsenalInventoryStorageManagerComponent.", LogLevel.WARNING);
			return;
		}

		// Resolve player entity
		IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!playerEntity)
			return;

		SCR_InventoryStorageManagerComponent invMgr =
			SCR_InventoryStorageManagerComponent.Cast(
				playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invMgr)
			return;

		SCR_LoadoutPreset preset = SCR_LoadoutPreset.Deserialize(presetData);
		if (!preset)
			return;

		// TODO: clear existing inventory before applying (entity deletion API not yet confirmed)
		ApplyLoadout(playerEntity, invMgr, arsenalStorage, preset);

		Rpc(Cl_OnApplied, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Cl_OnApplied(int playerID)
	{
		int localID = SCR_PlayerController.GetLocalPlayerId();
		if (playerID != localID)
			return;

		SCR_LoadoutEditorUI ui = SCR_LoadoutEditorUI.GetInstance();
		if (ui)
			ui.OnLoadoutApplied();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLoadout(IEntity player, SCR_InventoryStorageManagerComponent invMgr,
		SCR_ArsenalInventoryStorageManagerComponent arsenal, SCR_LoadoutPreset preset)
	{
		// Clothing first (establishes storage slots)
		TryEquipItem(invMgr, arsenal, preset.uniform);
		TryEquipItem(invMgr, arsenal, preset.vest);
		TryEquipItem(invMgr, arsenal, preset.backpack);
		TryEquipItem(invMgr, arsenal, preset.headgear);
		TryEquipItem(invMgr, arsenal, preset.facewear);

		// Weapons
		TryEquipItem(invMgr, arsenal, preset.primaryWeapon);
		TryEquipItem(invMgr, arsenal, preset.secondaryWeapon);
		TryEquipItem(invMgr, arsenal, preset.launcher);

		// Misc items
		foreach (string itemRN : preset.items)
			TryEquipItem(invMgr, arsenal, itemRN);
	}

	//------------------------------------------------------------------------------------------------
	protected void TryEquipItem(SCR_InventoryStorageManagerComponent invMgr,
		SCR_ArsenalInventoryStorageManagerComponent arsenal, string itemRN)
	{
		if (itemRN.IsEmpty())
			return;

		// Validate against the arsenal's available pool
		if (!arsenal.IsPrefabInArsenalStorage(itemRN))
			return;

		Resource res = Resource.Load(itemRN);
		if (!res || !res.IsValid())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		invMgr.GetOwner().GetWorldTransform(params.Transform);

		IEntity spawned = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (!spawned)
			return;

		invMgr.InsertItem(spawned, null, null, null);
	}

}
