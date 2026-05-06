// Client-side loadout editor panel.
// Extends ScriptedWidgetEventHandler so OnClick() intercepts button presses
// without needing a m_OnClick field on ButtonWidget.
class SCR_LoadoutEditorUI : ScriptedWidgetEventHandler
{
	protected static SCR_LoadoutEditorUI s_Instance;

	// Mod GUID from addon.gproj
	protected static const string LAYOUT_PATH = "{694AB44152191519}UI/layouts/LoadoutEditorMenu.layout";

	protected SCR_ArsenalInventoryStorageManagerComponent m_Arsenal;
	protected IEntity                                      m_PlayerEntity;
	protected Widget                                       m_Root;
	protected ref SCR_LoadoutPreset                        m_WorkingPreset;
	protected ref array<ref SCR_LoadoutPreset>             m_Presets;

	// Per-category item arrays populated from arsenal storage
	protected ref array<string> m_PrimaryItems;
	protected ref array<string> m_PrimaryNames;
	protected ref array<string> m_SecondaryItems;
	protected ref array<string> m_SecondaryNames;
	protected ref array<string> m_LauncherItems;
	protected ref array<string> m_LauncherNames;
	protected ref array<string> m_UniformItems;
	protected ref array<string> m_UniformNames;
	protected ref array<string> m_VestItems;
	protected ref array<string> m_VestNames;
	protected ref array<string> m_BackpackItems;
	protected ref array<string> m_BackpackNames;
	protected ref array<string> m_HeadgearItems;
	protected ref array<string> m_HeadgearNames;
	protected ref array<string> m_MiscItems;
	protected ref array<string> m_MiscNames;

	// Current selection indices per category
	protected int m_iPrimary;
	protected int m_iSecondary;
	protected int m_iLauncher;
	protected int m_iUniform;
	protected int m_iVest;
	protected int m_iBackpack;
	protected int m_iHeadgear;
	protected int m_iMisc;

	// Cached widgets
	protected TextWidget  m_TxtPrimary;
	protected TextWidget  m_TxtSecondary;
	protected TextWidget  m_TxtLauncher;
	protected TextWidget  m_TxtUniform;
	protected TextWidget  m_TxtVest;
	protected TextWidget  m_TxtBackpack;
	protected TextWidget  m_TxtHeadgear;
	protected TextWidget  m_TxtMisc;
	protected TextWidget  m_TxtPresetName;
	protected TextWidget  m_TxtStatus;
	protected EditBoxWidget m_InputPresetName;

	// Button widgets — used for identity checks in OnClick
	protected ButtonWidget m_BtnPriPrev;
	protected ButtonWidget m_BtnPriNext;
	protected ButtonWidget m_BtnSecPrev;
	protected ButtonWidget m_BtnSecNext;
	protected ButtonWidget m_BtnLncPrev;
	protected ButtonWidget m_BtnLncNext;
	protected ButtonWidget m_BtnUniPrev;
	protected ButtonWidget m_BtnUniNext;
	protected ButtonWidget m_BtnVstPrev;
	protected ButtonWidget m_BtnVstNext;
	protected ButtonWidget m_BtnBpkPrev;
	protected ButtonWidget m_BtnBpkNext;
	protected ButtonWidget m_BtnHdgPrev;
	protected ButtonWidget m_BtnHdgNext;
	protected ButtonWidget m_BtnMscPrev;
	protected ButtonWidget m_BtnMscNext;
	protected ButtonWidget m_BtnSave;
	protected ButtonWidget m_BtnLoad;
	protected ButtonWidget m_BtnDelete;
	protected ButtonWidget m_BtnApply;

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutEditorUI GetInstance() { return s_Instance; }

	//------------------------------------------------------------------------------------------------
	void Open(SCR_ArsenalInventoryStorageManagerComponent arsenal, IEntity playerEntity)
	{
		s_Instance     = this;
		m_Arsenal      = arsenal;
		m_PlayerEntity = playerEntity;
		m_WorkingPreset = new SCR_LoadoutPreset();
		m_Presets       = SCR_LoadoutPresetStorage.LoadAll();

		InitCategoryArrays();
		BuildItemLists();

		m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT_PATH);
		if (!m_Root)
		{
			Print("[LoadoutEditor] Open — layout not found: " + LAYOUT_PATH, LogLevel.ERROR);
			return;
		}

		// Handler is wired via the layout root widget's "HandlerClass" attribute (set in Workbench UI Editor)
		CacheWidgets();
		RefreshAllLabels();
		SetStatus("Ready.");
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		if (m_Root)
		{
			m_Root.RemoveFromHierarchy();
			m_Root = null;
		}
		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	void OnLoadoutApplied()
	{
		SetStatus("Loadout applied!");
	}

	//------------------------------------------------------------------------------------------------
	// ScriptedWidgetEventHandler — all button clicks routed here.
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w) return false;

		// Primary
		if (w == m_BtnPriPrev)  { CycleCategory(m_iPrimary,   m_PrimaryItems,   -1); UpdateLabel(m_TxtPrimary,   m_PrimaryNames,   m_iPrimary);   return true; }
		if (w == m_BtnPriNext)  { CycleCategory(m_iPrimary,   m_PrimaryItems,    1); UpdateLabel(m_TxtPrimary,   m_PrimaryNames,   m_iPrimary);   return true; }
		// Secondary
		if (w == m_BtnSecPrev)  { CycleCategory(m_iSecondary, m_SecondaryItems, -1); UpdateLabel(m_TxtSecondary, m_SecondaryNames, m_iSecondary); return true; }
		if (w == m_BtnSecNext)  { CycleCategory(m_iSecondary, m_SecondaryItems,  1); UpdateLabel(m_TxtSecondary, m_SecondaryNames, m_iSecondary); return true; }
		// Launcher
		if (w == m_BtnLncPrev)  { CycleCategory(m_iLauncher,  m_LauncherItems,  -1); UpdateLabel(m_TxtLauncher,  m_LauncherNames,  m_iLauncher);  return true; }
		if (w == m_BtnLncNext)  { CycleCategory(m_iLauncher,  m_LauncherItems,   1); UpdateLabel(m_TxtLauncher,  m_LauncherNames,  m_iLauncher);  return true; }
		// Uniform
		if (w == m_BtnUniPrev)  { CycleCategory(m_iUniform,   m_UniformItems,   -1); UpdateLabel(m_TxtUniform,   m_UniformNames,   m_iUniform);   return true; }
		if (w == m_BtnUniNext)  { CycleCategory(m_iUniform,   m_UniformItems,    1); UpdateLabel(m_TxtUniform,   m_UniformNames,   m_iUniform);   return true; }
		// Vest
		if (w == m_BtnVstPrev)  { CycleCategory(m_iVest,      m_VestItems,      -1); UpdateLabel(m_TxtVest,      m_VestNames,      m_iVest);      return true; }
		if (w == m_BtnVstNext)  { CycleCategory(m_iVest,      m_VestItems,       1); UpdateLabel(m_TxtVest,      m_VestNames,      m_iVest);      return true; }
		// Backpack
		if (w == m_BtnBpkPrev)  { CycleCategory(m_iBackpack,  m_BackpackItems,  -1); UpdateLabel(m_TxtBackpack,  m_BackpackNames,  m_iBackpack);  return true; }
		if (w == m_BtnBpkNext)  { CycleCategory(m_iBackpack,  m_BackpackItems,   1); UpdateLabel(m_TxtBackpack,  m_BackpackNames,  m_iBackpack);  return true; }
		// Headgear
		if (w == m_BtnHdgPrev)  { CycleCategory(m_iHeadgear,  m_HeadgearItems,  -1); UpdateLabel(m_TxtHeadgear,  m_HeadgearNames,  m_iHeadgear);  return true; }
		if (w == m_BtnHdgNext)  { CycleCategory(m_iHeadgear,  m_HeadgearItems,   1); UpdateLabel(m_TxtHeadgear,  m_HeadgearNames,  m_iHeadgear);  return true; }
		// Misc
		if (w == m_BtnMscPrev)  { CycleCategory(m_iMisc,      m_MiscItems,      -1); UpdateLabel(m_TxtMisc,      m_MiscNames,      m_iMisc);      return true; }
		if (w == m_BtnMscNext)  { CycleCategory(m_iMisc,      m_MiscItems,       1); UpdateLabel(m_TxtMisc,      m_MiscNames,      m_iMisc);      return true; }
		// Preset management
		if (w == m_BtnSave)   { OnSave();   return true; }
		if (w == m_BtnLoad)   { OnLoad();   return true; }
		if (w == m_BtnDelete) { OnDelete(); return true; }
		if (w == m_BtnApply)  { OnApply();  return true; }

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitCategoryArrays()
	{
		m_PrimaryItems    = new array<string>(); m_PrimaryNames    = new array<string>();
		m_SecondaryItems  = new array<string>(); m_SecondaryNames  = new array<string>();
		m_LauncherItems   = new array<string>(); m_LauncherNames   = new array<string>();
		m_UniformItems    = new array<string>(); m_UniformNames    = new array<string>();
		m_VestItems       = new array<string>(); m_VestNames       = new array<string>();
		m_BackpackItems   = new array<string>(); m_BackpackNames   = new array<string>();
		m_HeadgearItems   = new array<string>(); m_HeadgearNames   = new array<string>();
		m_MiscItems       = new array<string>(); m_MiscNames       = new array<string>();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildItemLists()
	{
		if (!m_Arsenal)
			return;

		array<IEntity> arsenalItems = new array<IEntity>();
		m_Arsenal.GetItems(arsenalItems);

		foreach (IEntity item : arsenalItems)
		{
			if (!item)
				continue;

			InventoryItemComponent ic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!ic)
				continue;

			UIInfo uiInfo = ic.GetUIInfo();
			string displayName = "Unknown";
			if (uiInfo)
				displayName = uiInfo.GetName();

			EntityPrefabData pd = item.GetPrefabData();
			if (!pd)
				continue;

			string rn = pd.GetPrefab();

			// Classify by item type using EquipedLoadoutStorageComponent / WeaponManager hints
			// Fallback: use display-name keywords for rough classification
			string lower = displayName;
			lower.ToLower();

			if (lower.Contains("pistol") || lower.Contains("handgun"))
			{ m_SecondaryItems.Insert(rn); m_SecondaryNames.Insert(displayName); }
			else if (lower.Contains("rifle") || lower.Contains("carbine") || lower.Contains("smg")
				|| lower.Contains("sniper") || lower.Contains("shotgun") || lower.Contains("lmg"))
			{ m_PrimaryItems.Insert(rn); m_PrimaryNames.Insert(displayName); }
			else if (lower.Contains("launcher") || lower.Contains("rpg") || lower.Contains("at4"))
			{ m_LauncherItems.Insert(rn); m_LauncherNames.Insert(displayName); }
			else if (lower.Contains("uniform") || lower.Contains("jacket") || lower.Contains("shirt"))
			{ m_UniformItems.Insert(rn); m_UniformNames.Insert(displayName); }
			else if (lower.Contains("vest") || lower.Contains("plate") || lower.Contains("carrier"))
			{ m_VestItems.Insert(rn); m_VestNames.Insert(displayName); }
			else if (lower.Contains("backpack") || lower.Contains("pack") || lower.Contains("bag"))
			{ m_BackpackItems.Insert(rn); m_BackpackNames.Insert(displayName); }
			else if (lower.Contains("helmet") || lower.Contains("hat") || lower.Contains("beret") || lower.Contains("cap"))
			{ m_HeadgearItems.Insert(rn); m_HeadgearNames.Insert(displayName); }
			else
			{ m_MiscItems.Insert(rn); m_MiscNames.Insert(displayName); }
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CacheWidgets()
	{
		m_TxtPrimary   = TextWidget.Cast(m_Root.FindAnyWidget("TxtPrimary"));
		m_TxtSecondary = TextWidget.Cast(m_Root.FindAnyWidget("TxtSecondary"));
		m_TxtLauncher  = TextWidget.Cast(m_Root.FindAnyWidget("TxtLauncher"));
		m_TxtUniform   = TextWidget.Cast(m_Root.FindAnyWidget("TxtUniform"));
		m_TxtVest      = TextWidget.Cast(m_Root.FindAnyWidget("TxtVest"));
		m_TxtBackpack  = TextWidget.Cast(m_Root.FindAnyWidget("TxtBackpack"));
		m_TxtHeadgear  = TextWidget.Cast(m_Root.FindAnyWidget("TxtHeadgear"));
		m_TxtMisc      = TextWidget.Cast(m_Root.FindAnyWidget("TxtMisc"));
		m_TxtStatus    = TextWidget.Cast(m_Root.FindAnyWidget("TxtStatus"));
		m_InputPresetName = EditBoxWidget.Cast(m_Root.FindAnyWidget("InputPresetName"));

		m_BtnPriPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnPriPrev"));
		m_BtnPriNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnPriNext"));
		m_BtnSecPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnSecPrev"));
		m_BtnSecNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnSecNext"));
		m_BtnLncPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnLncPrev"));
		m_BtnLncNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnLncNext"));
		m_BtnUniPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnUniPrev"));
		m_BtnUniNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnUniNext"));
		m_BtnVstPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnVstPrev"));
		m_BtnVstNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnVstNext"));
		m_BtnBpkPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnBpkPrev"));
		m_BtnBpkNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnBpkNext"));
		m_BtnHdgPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnHdgPrev"));
		m_BtnHdgNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnHdgNext"));
		m_BtnMscPrev = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnMscPrev"));
		m_BtnMscNext = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnMscNext"));
		m_BtnSave    = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnSave"));
		m_BtnLoad    = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnLoad"));
		m_BtnDelete  = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnDelete"));
		m_BtnApply   = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnApply"));
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshAllLabels()
	{
		UpdateLabel(m_TxtPrimary,   m_PrimaryNames,   m_iPrimary);
		UpdateLabel(m_TxtSecondary, m_SecondaryNames, m_iSecondary);
		UpdateLabel(m_TxtLauncher,  m_LauncherNames,  m_iLauncher);
		UpdateLabel(m_TxtUniform,   m_UniformNames,   m_iUniform);
		UpdateLabel(m_TxtVest,      m_VestNames,      m_iVest);
		UpdateLabel(m_TxtBackpack,  m_BackpackNames,  m_iBackpack);
		UpdateLabel(m_TxtHeadgear,  m_HeadgearNames,  m_iHeadgear);
		UpdateLabel(m_TxtMisc,      m_MiscNames,      m_iMisc);
	}

	//------------------------------------------------------------------------------------------------
	protected void CycleCategory(out int idx, array<string> items, int dir)
	{
		if (items.IsEmpty())
			return;

		idx = (idx + dir + items.Count()) % items.Count();

		// Sync selection to working preset
		SyncWorkingPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLabel(TextWidget lbl, array<string> names, int idx)
	{
		if (!lbl) return;
		if (names.IsEmpty()) { lbl.SetText("(none)"); return; }
		lbl.SetText(names[idx]);
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncWorkingPreset()
	{
		if (!m_WorkingPreset)
			return;

		if (!m_PrimaryItems.IsEmpty())   m_WorkingPreset.primaryWeapon   = m_PrimaryItems[m_iPrimary];
		if (!m_SecondaryItems.IsEmpty()) m_WorkingPreset.secondaryWeapon = m_SecondaryItems[m_iSecondary];
		if (!m_LauncherItems.IsEmpty())  m_WorkingPreset.launcher        = m_LauncherItems[m_iLauncher];
		if (!m_UniformItems.IsEmpty())   m_WorkingPreset.uniform         = m_UniformItems[m_iUniform];
		if (!m_VestItems.IsEmpty())      m_WorkingPreset.vest            = m_VestItems[m_iVest];
		if (!m_BackpackItems.IsEmpty())  m_WorkingPreset.backpack        = m_BackpackItems[m_iBackpack];
		if (!m_HeadgearItems.IsEmpty())  m_WorkingPreset.headgear        = m_HeadgearItems[m_iHeadgear];
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSave()
	{
		if (!m_InputPresetName || !m_WorkingPreset)
			return;

		string name = m_InputPresetName.GetText();
		if (name.IsEmpty()) { SetStatus("Enter a preset name."); return; }

		SyncWorkingPreset();
		m_WorkingPreset.name = name;

		if (SCR_LoadoutPresetStorage.SavePreset(m_WorkingPreset))
		{
			m_Presets = SCR_LoadoutPresetStorage.LoadAll();
			SetStatus("Saved: " + name);
		}
		else
			SetStatus("Save failed (limit reached?).");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLoad()
	{
		if (!m_InputPresetName)
			return;

		string name = m_InputPresetName.GetText();
		SCR_LoadoutPreset found = SCR_LoadoutPresetStorage.FindPreset(name);
		if (!found) { SetStatus("Preset not found: " + name); return; }

		m_WorkingPreset = SCR_LoadoutPreset.CreateCopy(found);
		SetStatus("Loaded: " + name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDelete()
	{
		if (!m_InputPresetName)
			return;

		string name = m_InputPresetName.GetText();
		if (SCR_LoadoutPresetStorage.DeletePreset(name))
		{
			m_Presets = SCR_LoadoutPresetStorage.LoadAll();
			m_WorkingPreset = new SCR_LoadoutPreset();
			SetStatus("Deleted: " + name);
		}
		else
			SetStatus("Not found: " + name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnApply()
	{
		if (!m_WorkingPreset)
			{ SetStatus("No preset to apply."); return; }

		SyncWorkingPreset();

		SCR_LoadoutEditorManager mgr = SCR_LoadoutEditorManager.GetInstance();
		if (!mgr)
			{ SetStatus("Manager not found."); return; }

		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_PlayerEntity);

		RplComponent arsenalRpl = RplComponent.Cast(m_Arsenal.GetOwner().FindComponent(RplComponent));
		if (!arsenalRpl)
			{ SetStatus("Arsenal not replicated."); return; }

		mgr.RequestApplyLoadout(playerID, m_WorkingPreset.Serialize(), arsenalRpl.Id());
		SetStatus("Applying...");
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStatus(string msg)
	{
		if (m_TxtStatus)
			m_TxtStatus.SetText(msg);
	}
}
