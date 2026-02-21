import gfx.managers.FocusHandler;
import gfx.io.GameDelegate;
import gfx.ui.InputDetails;
import Components.CrossPlatformButtons;
import Components.Meter;
import Shared.GlobalFunc;
import gfx.ui.NavigationCode;
import skse;

class CharacterSheet extends MovieClip
{
	#include "../version.as"
	
	static var INFO_IDX = 0;
	static var SKILLS_IDX = 1;
	static var FACTIONS_IDX = 2;
	static var STATS_IDX = 3;
	
	var CharacterSheet_mc:MovieClip;
	var LevelMeter_mc:MovieClip;
	var LevelMeter:MovieClip;
	var MenuHeader_mc:MovieClip;
	var MenuHeader:MovieClip;
	var BaseInfo_mc:MovieClip;
	var PlayerInfoInstance_mc:MovieClip;
	var PlayerInfoInstance:MovieClip;
	var CategoryTitle_mc:MovieClip;
	var CategoryTitle:MovieClip;
	var InfoHolder_mc:MovieClip;
	var InfoHolder:MovieClip;
	var SkillsHolder_mc:MovieClip;
	var SkillsHolder:MovieClip;
	var FactionsHolder_mc:MovieClip;
	var FactionsHolder:MovieClip;
	var StatsHolder_mc:MovieClip;
	var StatsHolder:MovieClip;
	var MiscHolder_mc:MovieClip;
	var MiscHolder:MovieClip;
	var SkillDetails_mc:MovieClip;
	var SkillDetails:MovieClip;
	var CharDetails_mc:MovieClip;
	var CharDetails:MovieClip;
	var MenuLeftSide_mc:MovieClip;
	var magickaMeter:MovieClip;
	var healthMeter:MovieClip;
	var staminaMeter:MovieClip;
	var SkillsMask_mc:MovieClip;
	var SkillsMask:MovieClip;
	var FactionsMask_mc:MovieClip;
	var FactionsMask:MovieClip;
	var DebugText_mc:MovieClip;
	var debugTextField;
	
	var bGamepad:Boolean;
	var bUpdated:Boolean;
	var bOpeningMenu:Boolean;
	var bShowDetails:Boolean;
	var iCurrentPage:Number;
	var iCurrentInfoIndex:Number;
	var iCurrentSkillIndex:Number;
	var iCurrentFactionIndex:Number;
	var scrollAmount:Number = 40;
	var skillsListHeight:Number;
	var factionsListHeight:Number;
	
	var Categories = new Array();
	var Pages = new Array();
	var aInfoClipsContainer = new Array();
	var aSkillClipsContainer = new Array();
	var aFactionClipsContainer = new Array();
	
	var skillsMenuButton:MovieClip;
	var skillsMenuText:MovieClip;
	var showDetailButton:MovieClip;
	var	showDetailText:MovieClip;
	var changeTitleButton:MovieClip;
	var changeTitleText:MovieClip;
	
	private var _platform:Number;

	private var _deleteControls:Object;
	private var _defaultControls:Object;
	private var _kinectControls:Object;
	private var _acceptControls:Object;
	private var _cancelControls:Object;
	private var _acceptButton:MovieClip;
	private var _cancelButton:MovieClip;
	private var _currentFocus:MovieClip;
	
	function CharacterSheet()
	{
		super();
		FocusHandler.instance.setFocus(this,0);
		Mouse.addListener(this);
		debugTextField = DebugText_mc.debugTextField;
		
		InfoHolder = MenuLeftSide_mc.InfoHolder_mc;
		SkillDetails = MenuLeftSide_mc.SkillDetails_mc;
		CharDetails = MenuLeftSide_mc.CharDetails_mc;
		SkillsHolder = MenuLeftSide_mc.SkillsHolder_mc;
		StatsHolder = MenuLeftSide_mc.StatsHolder_mc;
		FactionsHolder = MenuLeftSide_mc.FactionsHolder_mc;
		SkillsMask = MenuLeftSide_mc.SkillsMask_mc;
		FactionsMask = MenuLeftSide_mc.FactionsMask_mc;
		CategoryTitle = MenuLeftSide_mc.CategoryTitle_mc;
		LevelMeter = BaseInfo_mc.LevelMeter_mc;
		MenuHeader = MenuLeftSide_mc.MenuHeader_mc;
		PlayerInfoInstance = BaseInfo_mc.PlayerInfoInstance_mc.PlayerInfoCardInstance;
		MiscHolder = BaseInfo_mc.MiscHolder_mc;
		magickaMeter = PlayerInfoInstance.MagickaMeterInstance.MagickaMeter_mc;
		healthMeter = PlayerInfoInstance.HealthMeterInstance.HealthMeter_mc;
		staminaMeter = PlayerInfoInstance.StaminaMeterInstance.StaminaMeter_mc;
		
		InfoHolder.name.title.text = "$NAME";
		InfoHolder.name.highlight._visible = false;
		InfoHolder.name.index = 0;
		InfoHolder.race.title.text = "$RACE";
		InfoHolder.race.highlight._visible = false;
		InfoHolder.race.index = 1;
		InfoHolder.constellation.title.text = "$CONSTELLATION";
		InfoHolder.constellation.highlight._visible = false;
		InfoHolder.constellation.index = 2;
		InfoHolder.attrClass.title.text = "$CLASS";
		InfoHolder.attrClass.highlight._visible = false;
		InfoHolder.attrClass.index = 3;
		MenuHeader.playerTitle.text = "";
		
		SkillDetails._visible = false;
		CharDetails._visible = false;
		showDetailText.text = "$DETAIL_BUTTON_SHOW";
		skillsMenuText.text = "$OPEN_SKILLS_MENU";
		skillsMenuText._visible = false;
		skillsMenuButton._visible = false;
		changeTitleText.text = "$CHANGE_PLAYER_TITLE";
		changeTitleText._visible = false;
		changeTitleButton._visible = false;
		SkillDetails.description.textAutoSize = "shrink";
		SkillDetails.description.verticalAlign = "center";
		CharDetails.description.textAutoSize = "shrink";
		CharDetails.description.verticalAlign = "center";
		
		SkillsHolder.setMask(SkillsMask);
		SkillsMask._visible = false;
		FactionsHolder.setMask(FactionsMask);
		FactionsMask._visible = false;
		
		iCurrentPage = INFO_IDX;
		iCurrentInfoIndex = -1;
		iCurrentSkillIndex = -1;
		iCurrentFactionIndex = -1;
		_platform = 0;
		bUpdated = false;
		bOpeningMenu = false;
		bShowDetails = false;
	}
	
	function onLoad()
	{
		Pages.push({page: InfoHolder, index: INFO_IDX, text: "$CHARACTER"});
		Pages.push({page: SkillsHolder, index: SKILLS_IDX, text: "$SKILLS"});
		Pages.push({page: FactionsHolder, index: FACTIONS_IDX, text: "$FACTIONS"});
		Pages.push({page: StatsHolder, index: STATS_IDX, text: "$STATS"});
		Pages.sortOn("index");

		CategoryTitle.category1.textField.text = Pages[INFO_IDX].text;
		CategoryTitle.category1.index = INFO_IDX;
		CategoryTitle.category1.mask._alpha = 0;
		CategoryTitle.category1.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onCategoryHover(this._parent);
		};
		CategoryTitle.category1.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onCategoryRollOut(this._parent);
		};
		CategoryTitle.category1.mask.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				_parent._parent._parent._parent.onCategoryClick(this._parent);
			}
		};
		CategoryTitle.category2.textField.text = Pages[SKILLS_IDX].text;
		CategoryTitle.category2.index = SKILLS_IDX;
		CategoryTitle.category2.mask._alpha = 0;
		CategoryTitle.category2.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onCategoryHover(this._parent);
		};
		CategoryTitle.category2.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onCategoryRollOut(this._parent);
		};
		CategoryTitle.category2.mask.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				_parent._parent._parent._parent.onCategoryClick(this._parent);
			}
		};
		CategoryTitle.category3.textField.text = Pages[FACTIONS_IDX].text;
		CategoryTitle.category3.index = FACTIONS_IDX;
		CategoryTitle.category3.mask._alpha = 0;
		CategoryTitle.category3.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onCategoryHover(this._parent);
		};
		CategoryTitle.category3.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onCategoryRollOut(this._parent);
		};
		CategoryTitle.category3.mask.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				_parent._parent._parent._parent.onCategoryClick(this._parent);
			}
		};
		CategoryTitle.category4.textField.text = Pages[STATS_IDX].text;
		CategoryTitle.category4.index = STATS_IDX;
		CategoryTitle.category4.mask._alpha = 0;
		CategoryTitle.category4.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onCategoryHover(this._parent);
		};
		CategoryTitle.category4.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onCategoryRollOut(this._parent);
		};
		CategoryTitle.category4.mask.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				_parent._parent._parent._parent.onCategoryClick(this._parent);
			}
		};
		Categories.push(CategoryTitle.category1);
		Categories.push(CategoryTitle.category2);
		Categories.push(CategoryTitle.category3);
		Categories.push(CategoryTitle.category4);
		
		for (var i = 0; i < Pages.length; i++){
			if (i == iCurrentPage){
				Pages[i].page._visible = true;
				Categories[i].selector._visible = true;
				Categories[i].textField.textColor = 0xffffff;
				Categories[i].textField._alpha = 100;
			} else {
				Pages[i].page._visible = false;
				Categories[i].selector._visible = false;
				Categories[i].textField.textColor = 0xbcbcbc;
				Categories[i].textField._alpha = 45;
				
			}
		}
		
		aInfoClipsContainer.push(InfoHolder.name);
		aInfoClipsContainer.push(InfoHolder.race);
		aInfoClipsContainer.push(InfoHolder.constellation);
		aInfoClipsContainer.push(InfoHolder.attrClass);
		
		SetPlatform(_platform, false);
		GameDelegate.call("PlaySound", ["UIJournalOpen"]);
	}
	
	function handleInput(details:InputDetails, pathToFocus:Array):Boolean
	{
		var bHandledInput:Boolean = false;
		if (GlobalFunc.IsKeyPressed(details))
		{
			if (details.navEquivalent == NavigationCode.TAB || details.navEquivalent == NavigationCode.GAMEPAD_B) {
				CloseMenu();
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.GAMEPAD_L1 || details.code == 81)
			{
				var nextPage;
				if (iCurrentPage == 0){
					nextPage = Pages.length - 1;
				} else {
					nextPage = iCurrentPage - 1;
				}
				ChangePage(nextPage);
				GameDelegate.call("PlaySound",["UIMenuPrevNext"]);
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.GAMEPAD_R1 || details.code == 82)
			{
				var nextPage;
				if (iCurrentPage >= Pages.length - 1){
					nextPage = 0;
				} else {
					nextPage = iCurrentPage + 1;
				}
				ChangePage(nextPage);
				GameDelegate.call("PlaySound",["UIMenuPrevNext"]);
				bHandledInput = true;
			}
			else if (details.code == 71 || details.navEquivalent == NavigationCode.GAMEPAD_Y)
			{
				if (skillsMenuButton._visible) {
				  var self = this;
				  var after:Number = setTimeout(function() {
					  GameDelegate.call("OpenSkillsMenu", []);
				  }, 300);
				
				  skyui.util.Tween.LinearTween(this, "_alpha", this._alpha, 0, 0.25);
				}
				else if (changeTitleButton._visible) {
					ChangePlayerTitle();
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.UP)
			{
				if (iCurrentPage == SKILLS_IDX){
					if (iCurrentSkillIndex > 0)
					{
						onSkillHover(aSkillClipsContainer[iCurrentSkillIndex - 1]);
						if (iCurrentSkillIndex == 0)
						{
							SkillsHolder._y = - 220;
						} 
						else if (aSkillClipsContainer[iCurrentSkillIndex]._y + SkillsHolder._y < - 220)
						{
							SkillsHolder._y = - 220 - aSkillClipsContainer[iCurrentSkillIndex]._y;
						}
					}
				}
				else if (iCurrentPage == FACTIONS_IDX){
					if (iCurrentFactionIndex > 0)
					{
						onFactionHover(aFactionClipsContainer[iCurrentFactionIndex - 1]);
						if (iCurrentFactionIndex == 0)
						{
							FactionsHolder._y = - 204;
						} 
						else if (aFactionClipsContainer[iCurrentFactionIndex]._y + FactionsHolder._y < - 204)
						{
							FactionsHolder._y = - 204 - aFactionClipsContainer[iCurrentFactionIndex]._y;
						}
					}
				}
				else if (iCurrentPage == INFO_IDX){
					if (iCurrentInfoIndex > 0){
						onInfoHover(aInfoClipsContainer[iCurrentInfoIndex - 1])
					}
					if (iCurrentInfoIndex < 1){
						CharDetails._visible = false;
					}
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.DOWN)
			{
				if (iCurrentPage == SKILLS_IDX){
					if (iCurrentSkillIndex < aSkillClipsContainer.length - 1)
					{
						onSkillHover(aSkillClipsContainer[iCurrentSkillIndex + 1]);
						if (aSkillClipsContainer[iCurrentSkillIndex]._y + SkillsHolder._y > 185)
						{
							SkillsHolder._y = 185 - aSkillClipsContainer[iCurrentSkillIndex]._y;
						}
						if (bShowDetails && iCurrentSkillIndex > -1){
							SkillDetails._visible = true;
						}
					}
				}
				else if (iCurrentPage == FACTIONS_IDX){
					if (iCurrentFactionIndex < aFactionClipsContainer.length - 1)
					{
						onFactionHover(aFactionClipsContainer[iCurrentFactionIndex + 1]);
						if (aFactionClipsContainer[iCurrentFactionIndex]._y + FactionsHolder._y > 175)
						{
							FactionsHolder._y = 175 - aFactionClipsContainer[iCurrentFactionIndex]._y;
						}
						changeTitleText._visible = true;
						changeTitleButton._visible = true;
					}
				}
				else if (iCurrentPage == INFO_IDX){
					if (iCurrentInfoIndex < 3){
						onInfoHover(aInfoClipsContainer[iCurrentInfoIndex + 1])
					}
					if (bShowDetails && iCurrentInfoIndex > 0){
						CharDetails._visible = true;
					}
				}
				bHandledInput = true;
			}
			else if (details.code == 70 || details.navEquivalent == NavigationCode.GAMEPAD_R3)
			{
				bShowDetails = !bShowDetails;
				
				if (bShowDetails == true){
					showDetailText.text = "$DETAIL_BUTTON_HIDE";
					GameDelegate.call("PlaySound",["UIMenuBladeOpenSD"]);
				} else {
					GameDelegate.call("PlaySound",["UIMenuBladeCloseSD"]);
					showDetailText.text = "$DETAIL_BUTTON_SHOW";
				}
				
				if(iCurrentSkillIndex > -1 && iCurrentPage == SKILLS_IDX){				
					if (SkillDetails._visible == false)
					{
						SetSkillDetails(aSkillClipsContainer[iCurrentSkillIndex]);
						SkillDetails._visible = true;
					} else {
						SkillDetails._visible = false;
					}
				}
				else if (iCurrentInfoIndex > 0 && iCurrentPage == INFO_IDX){
					if (CharDetails._visible == false){
						SetCharDetails(aInfoClipsContainer[iCurrentInfoIndex]);
						CharDetails._visible = true;
					} else {
						CharDetails._visible = false;
					}
				}
				bHandledInput = true;
			}
		}
		return bHandledInput;
	}
	
	function CloseMenu(moveLeft): Void
	{
		GameDelegate.call("CloseMenu", []);
	}
	
	function onMouseWheel(delta:Number):Void
	{
		var pt:Object = {_x:_xmouse, _y:_ymouse};
		var tolerance:Number = 0.1;// Tolerance for floating-point comparison
		var minScrollY:Number;

		if (iCurrentPage == SKILLS_IDX)
		{
			if (pt._x >= SkillsMask._x && pt._x <= SkillsMask._x + SkillsMask._width && pt._y >= SkillsMask._y && pt._y <= SkillsMask._y + SkillsMask._height)
			{
				minScrollY = SkillsMask._y + SkillsMask._height - skillsListHeight + 11;
				if ((Math.abs(SkillsHolder._y - (SkillsMask._y - 39)) < tolerance && delta > 0) || (skillsListHeight - SkillsMask._height < tolerance && delta < 0))
				{
					return;
				}
				else
				{
					SkillsHolder._y += delta * scrollAmount;

					if (SkillsHolder._y > SkillsMask._y - 39)
					{
						SkillsHolder._y = SkillsMask._y - 39;
					}

					if (SkillsHolder._y < minScrollY)
					{
						SkillsHolder._y = minScrollY;
					}
				}
				return;
			}
		}
		else if (iCurrentPage == FACTIONS_IDX){
			if (pt._x >= FactionsMask._x && pt._x <= FactionsMask._x + FactionsMask._width && pt._y >= FactionsMask._y && pt._y <= FactionsMask._y + FactionsMask._height)
			{
				minScrollY = FactionsMask._y + FactionsMask._height - factionsListHeight + 63;
				if ((Math.abs(FactionsHolder._y - (FactionsMask._y - 26)) < tolerance && delta > 0) || (factionsListHeight - FactionsMask._height < tolerance && delta < 0))
				{
					return;
				}
				else
				{
					FactionsHolder._y += delta * scrollAmount;

					if (FactionsHolder._y > FactionsMask._y - 26)
					{
						FactionsHolder._y = FactionsMask._y - 26;
					}

					if (FactionsHolder._y < minScrollY)
					{
						FactionsHolder._y = minScrollY;
					}
				}
				return;
			}
		}
		return;
	}
	
	function onCategoryHover(category:MovieClip): Void{
		if (category.index == iCurrentPage){
			return;
		} else {
			category.textField.textColor = 0xffffff;
			category.textField._alpha = 100;
		}
	}
	
	function onCategoryRollOut(category:MovieClip): Void{
		if (category.index == iCurrentPage){
			return;
		} else {
			category.textField.textColor = 0xbcbcbc;
			category.textField._alpha = 45;
		}
	}
	
	function onCategoryClick(category:MovieClip): Void{
		if (category.index == iCurrentPage){
			return;
		} else {
			ChangePage(category.index);
		}
	}
	
	function ChangePage(pageNum){
		Pages[iCurrentPage].page._visible = false;
		Categories[iCurrentPage].selector._visible = false;
		Categories[iCurrentPage].textField.textColor = 0xbcbcbc;
		Categories[iCurrentPage].textField._alpha = 45;
		iCurrentPage = pageNum;
		if(bShowDetails){
			switch (iCurrentPage){
				case INFO_IDX:
					if(iCurrentInfoIndex > 0){
						CharDetails._visible = true;
					} else {
						CharDetails._visible = false;
					}
					SkillDetails._visible = false;
					break;
				case SKILLS_IDX:
					CharDetails._visible = false;
					if(iCurrentSkillIndex > -1){
						SkillDetails._visible = true;
					} else {
						SkillDetails._visible = false;
					}
					break;
				case FACTIONS_IDX:
					CharDetails._visible = false;
					SkillDetails._visible = false;
				case STATS_IDX:
					CharDetails._visible = false;
					SkillDetails._visible = false;
					break;
				default:
					break;
			}
		} else {
			CharDetails._visible = false;
			SkillDetails._visible = false;
		}
		
		if (iCurrentPage == SKILLS_IDX){
			skillsMenuButton._visible = true;
			skillsMenuText._visible = true;
			changeTitleButton._visible = false;
			changeTitleText._visible = false;
		} else if (iCurrentPage == FACTIONS_IDX && iCurrentFactionIndex > -1){
			changeTitleButton._visible = true;
			changeTitleText._visible = true;
			skillsMenuButton._visible = false;
			skillsMenuText._visible = false;
		} 
		else {
			skillsMenuButton._visible = false;
			skillsMenuText._visible = false;
			changeTitleButton._visible = false;
			changeTitleText._visible = false;
		}
		
		Pages[iCurrentPage].page._visible = true;
		Categories[iCurrentPage].selector._visible = true;
		Categories[iCurrentPage].textField.textColor = 0xffffff;
		Categories[iCurrentPage].textField._alpha = 100;
	}
	
	function SetGenericData(playerName, race, level, xpProgress, datetime, constellation, raceDescription, constellationDescription, condition): Void
	{
		MenuHeader.CharacterName.text = playerName.toUpperCase();
		MenuHeader.CharacterName.autoSize = "left";
		MenuHeader.playerTitle._x = MenuHeader.CharacterName._x + MenuHeader.CharacterName._width + 30;
		//MenuHeader.DateText.text = datetime;
		LevelMeter.LevelNumberLabel.text = level;
		LevelMeter.LevelNumberLabel.textAutoSize = "shrink";
		LevelMeter.LevelProgressBar.gotoAndStop(140 - xpProgress);
		InfoHolder.name.info.text = playerName;
		InfoHolder.name.mask._alpha = 0;
		InfoHolder.name.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onInfoHover(this._parent);
		};
		InfoHolder.name.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onInfoRollOut(this._parent);
		};
		InfoHolder.race.info.text = race;
		InfoHolder.race.descTitle = race;
		InfoHolder.race.description = raceDescription;
		InfoHolder.race.spec = "";
		InfoHolder.race.condition = condition;
		InfoHolder.race.mask._alpha = 0;
		InfoHolder.race.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onInfoHover(this._parent);
		};
		InfoHolder.race.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onInfoRollOut(this._parent);
		};
		InfoHolder.constellation.info.text = constellation;
		InfoHolder.constellation.descTitle = constellation;
		InfoHolder.constellation.description = constellationDescription;
		InfoHolder.constellation.spec = "";
		InfoHolder.constellation.mask._alpha = 0;
		InfoHolder.constellation.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onInfoHover(this._parent);
		};
		InfoHolder.constellation.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onInfoRollOut(this._parent);
		};
		
		MenuHeader.playerTitleRank._x = InfoHolder.name.title.textWidth + 20 + MenuHeader.playerTitleRank._width;
		MenuHeader.playerTitleConnector._x = MenuHeader.playerTitleRank._x + MenuHeader.playerTitleRank._width;
		MenuHeader.playerTitleFaction._x = MenuHeader.playerTitleConnector._x + MenuHeader.playerTitleConnector._width;
	}
	
	function SetCharDetails(infoClip:MovieClip){
		CharDetails.title.text = infoClip.descTitle;
		CharDetails.description.text = infoClip.description;
		
		var condition = infoClip.condition;
		if (condition == "" || condition == undefined){
			CharDetails.playerCondition._visible = false;
		} else {
			CharDetails.playerCondition.gotoAndStop(condition);
			CharDetails.playerCondition._visible = true;
		}
		
		var specialization = infoClip.spec;
		if (specialization == ""){
			CharDetails.specialization.text = "";
			CharDetails.specTitle._visible = false;
			//CharDetails.description._y = -30;
			return;
		}
		
		//CharDetails.description._y = -9;
		CharDetails.specialization.text = specialization;
		CharDetails.specTitle._visible = true;
		if (specialization == "$Magic"){
			CharDetails.specialization.textColor = 0x4b80d6;
		} else if (specialization == "$Combat"){
			CharDetails.specialization.textColor = 0xd64b4b;
		} else {
			CharDetails.specialization.textColor = 0x4bd660;
		}
	}
	
	function SetSkillDetails(skillClip:MovieClip){
		SkillDetails.title.text = skillClip.name;
		SkillDetails.description.text = skillClip.description;
		SkillDetails.description.textAutoSize = "shrink";
		SkillDetails.level.text = skillClip.lvlText;
		SkillDetails.xpBar.gotoAndStop(skillClip.xpFrame);
		SkillDetails.skillIcon.gotoAndStop(skillClip.key);
	}
	
	function SetAttributesMeters(health, maxHealth, temporaryHealth, magicka, maxMagicka, temporaryMagicka, stamina, maxStamina, temporaryStamina): Void
	{
		SetMeter(healthMeter, PlayerInfoInstance.healthValue, health, maxHealth, temporaryHealth);
		SetMeter(magickaMeter, PlayerInfoInstance.magicValue, magicka, maxMagicka, temporaryMagicka);
		SetMeter(staminaMeter, PlayerInfoInstance.enduranceValue, stamina, maxStamina, temporaryStamina);
	}
	
	function SetMeter(meter, textArea, currentValue, maxValue, modifier): Void
	{
		var meterPercent: Number = 100 * (Math.max(0, Math.min(currentValue, maxValue)) / maxValue);
			
		meter.gotoAndStop(200 - 2*meterPercent);
		textArea.text = Math.floor(currentValue) + "/" + maxValue;
		if (modifier > 0) {
			textArea.textColor = 0x1C8F16;
		} else if (modifier < 0){
			textArea.textColor = 0xFB0000;
		}
	}
	
	function SetSkills(skillsData, attributedClass, classSpecialization, classDescription): Void{
		alphabetSort(skillsData, "skillName");
		//numericSort(skillsData, "level", true);
		
		for (var i = 0; i < skillsData.length; i++)
		{
			var skill = skillsData[i];
			var offset = 45 * i;
			//debugLog("Skill name: "+skill.skillName+", offset "+offset);
			var skillClip:MovieClip = SkillsHolder.attachMovie("skillSelector", "skill" + i.toString(), SkillsHolder.getNextHighestDepth(), {_x:0, _y:offset});
			skillClip.textField.text = skill.skillName;
			skillClip.name = skill.skillName;
			skillClip.description = skill.description;
			skillClip.lvlText = skill.level;
			skillClip.level.text = skill.level;
			skillClip.key = skill.key;
			skillClip.xpFrame = skill.xpFrame;
			skillClip.highlight._visible = false;
			skillClip.mask._alpha = 0;
			skillClip.mask.onRollOver = function()
			{
				_parent._parent._parent._parent.onSkillHover(this._parent);
			};
			skillClip.mask.onRollOut = function()
			{
				_parent._parent._parent._parent.onSkillRollOut(this._parent);
			};
			
			skillClip.index = aSkillClipsContainer.length;
			aSkillClipsContainer.push(skillClip);
		}
		InfoHolder.attrClass.info.text = attributedClass;
		InfoHolder.attrClass.descTitle = attributedClass;
		InfoHolder.attrClass.description = classDescription;
		InfoHolder.attrClass.spec = classSpecialization;
		InfoHolder.attrClass.mask._alpha = 0;
		InfoHolder.attrClass.mask.onRollOver = function()
		{
			_parent._parent._parent._parent.onInfoHover(this._parent);
		};
		InfoHolder.attrClass.mask.onRollOut = function()
		{
			_parent._parent._parent._parent.onInfoRollOut(this._parent);
		};
		skillsListHeight = ((skillsData.length + 1) * 45) - 5;
	}
	
	function onSkillHover(skillSelector:MovieClip){
		if (iCurrentPage == SKILLS_IDX){
			onSkillRollOut(aSkillClipsContainer[iCurrentSkillIndex]);
			iCurrentSkillIndex = skillSelector.index;
			skillSelector.highlight._visible = true;
			SetSkillDetails(skillSelector);
			if (bShowDetails){
				SkillDetails._visible = true;
			}
		}
	}
	
	function onSkillRollOut(skillSelector:MovieClip){
		if (iCurrentPage == SKILLS_IDX){
			skillSelector.highlight._visible = false;
		}
	}
	
	function onInfoHover(infoHolder:MovieClip){
		if (iCurrentPage == INFO_IDX){
			onInfoRollOut(aInfoClipsContainer[iCurrentInfoIndex])
			iCurrentInfoIndex = infoHolder.index;
			infoHolder.highlight._visible = true;
			SetCharDetails(infoHolder);
			if(bShowDetails && iCurrentInfoIndex > 0){
				CharDetails._visible = true;
			}
		}
	}
	
	function onInfoRollOut(infoHolder:MovieClip){
		if (iCurrentPage == INFO_IDX){
			infoHolder.highlight._visible = false;
			CharDetails._visible = false;
		}
	}
	
	function onFactionHover(factionHolder:MovieClip){
		if (iCurrentPage == FACTIONS_IDX){
			onFactionRollOut(aFactionClipsContainer[iCurrentFactionIndex]);
			iCurrentFactionIndex = factionHolder.index;
			factionHolder.highlight._visible = true;
		}
		changeTitleText._visible = true;
		changeTitleButton._visible = true;
	}
	
	function onFactionRollOut(factionHolder:MovieClip){
		if (iCurrentPage == FACTIONS_IDX){
			factionHolder.highlight._visible = false;
		}
		changeTitleText._visible = false;
		changeTitleButton._visible = false;
	}
	
	function SetFactions(factionsData): Void{
		for (var i in FactionsHolder)
		{
			FactionsHolder[i].removeMovieClip();
		}
		if (factionsData.length == 0){
			var factionClip:MovieClip = FactionsHolder.attachMovie("factionSelector", "faction" + i.toString(), FactionsHolder.getNextHighestDepth(), {_x:0, _y:offset});
			factionClip.textField.text = "$NO_FACTION";
			factionClip.rank.text = "";
			factionClip.icon._visible = false;
			factionClip.highlight._visible = false;
			factionClip.mask._visible = 0;
			factionClip.separator._visible = false;
			MenuHeader.playerTitleRank._visible = false;
			MenuHeader.playerTitleConnector._visible = false;
			MenuHeader.playerTitleFaction._visible = false;
		} else {
			for (var i = 0; i < factionsData.length; i++)
			{
				var offset = 75 * i;
				var faction = factionsData[i];
				//debugLog("Faction: "+faction.factionName);
				var factionClip:MovieClip = FactionsHolder.attachMovie("factionSelector", "faction" + i.toString(), FactionsHolder.getNextHighestDepth(), {_x:0, _y:offset});
				factionClip.textField.text = faction.factionName;
				factionClip.factionName = faction.factionName;
				factionClip.rank.text = faction.rank;
				factionClip.rank = faction.rank;
				factionClip.rankOnly = faction.rankDisplayOnly;
				factionClip.icon.gotoAndStop(faction.id);
				factionClip.highlight._visible = false;
				factionClip.mask._alpha = 0;
				factionClip.mask.onRollOver = function()
				{
					_parent._parent._parent._parent.onFactionHover(this._parent);
				};
				factionClip.mask.onRollOut = function()
				{
					_parent._parent._parent._parent.onFactionRollOut(this._parent);
				};
				
				factionClip.index = aFactionClipsContainer.length;
				aFactionClipsContainer.push(factionClip);
			}
			SetPlayerTitle(factionsData[0].rank, factionsData[0].factionName, factionsData[0].rankDisplayOnly);
		}
		factionsListHeight = ((factionsData.length + 1) * 75) - 5;
	}
	
	function SetPlayerTitle(rank, faction, rankOnly): Void{
		if (rank == "" || faction == ""){
			if (aFactionClipsContainer[0].rank == undefined || aFactionClipsContainer[0].factionName == undefined){
				MenuHeader.playerTitleRank._visible = false;
				MenuHeader.playerTitleConnector._visible = false;
				MenuHeader.playerTitleFaction._visible = false;
				return;
			}
			MenuHeader.playerTitleRank.text = aFactionClipsContainer[0].rank;
			MenuHeader.playerTitleFaction.text = aFactionClipsContainer[0].factionName;
		} else {
			MenuHeader.playerTitleRank.text = rank;
			MenuHeader.playerTitleFaction.text = faction;
		}
		
		if (rankOnly){
			MenuHeader.playerTitleConnector._visible = false;
			MenuHeader.playerTitleFaction._visible = false;
		} else {
			MenuHeader.playerTitleConnector._visible = true;
			MenuHeader.playerTitleFaction._visible = true;
			MenuHeader.playerTitleConnector.text = "$TITLE_CONNECTOR";
			MenuHeader.playerTitleRank.textAutoSize = "shrink";
			MenuHeader.playerTitleConnector.textAutoSize = "shrink";
			MenuHeader.playerTitleFaction.textAutoSize = "shrink";
		}
	}
	
	function ChangePlayerTitle(){
		GameDelegate.call("SaveFactionTitle", [aFactionClipsContainer[iCurrentFactionIndex].rank, aFactionClipsContainer[iCurrentFactionIndex].factionName]);
		SetPlayerTitle(aFactionClipsContainer[iCurrentFactionIndex].rank, aFactionClipsContainer[iCurrentFactionIndex].factionName, aFactionClipsContainer[iCurrentFactionIndex].rankOnly);
	}
	
	function SetStats(healRate, magickaRate, staminaRate, speedMult, weaponSpeedMult, critChance, poisonResist, magicResist, fireResist, frostResist, shockResist, diseaseResist): Void{
		StatsHolder.healRateValue.text = Math.floor(healRate);
		StatsHolder.magickaRateValue.text = Math.floor(magickaRate);
		StatsHolder.staminaRateValue.text = Math.floor(staminaRate);
		StatsHolder.speedValue.text = speedMult;
		StatsHolder.weaponSpeedValue.text = weaponSpeedMult;
		StatsHolder.magicResistValue.text = magicResist;
		StatsHolder.fireResistValue.text = fireResist;
		StatsHolder.frostResistValue.text = frostResist;
		StatsHolder.shockResistValue.text = shockResist;
		StatsHolder.poisonResistValue.text = poisonResist;
		StatsHolder.diseaseResistValue.text = diseaseResist;
	}
	
	function SetMiscData(gold, armor, carryWeight, maxCarryWeight, warmth): Void{
		MiscHolder.gold.text = gold;
		MiscHolder.armor.text = armor;
		MiscHolder.carryWeight.text = Math.ceil(carryWeight) + "/" + maxCarryWeight;
		if (warmth > -1){
			MiscHolder.warmth._visible = true;
			MiscHolder.warmthIcon._visible = true;
			MiscHolder.warmth.text = warmth;
		} else {
			MiscHolder.warmth._visible = false;
			MiscHolder.warmthIcon._visible = false;
		}
	}
	
	function SetGamepad(gamepad)
	{
		bGamepad = gamepad;
		if (!bGamepad){
			CategoryTitle.moveLeft.gotoAndStop(16); //Q
			CategoryTitle.moveRight.gotoAndStop(19);	//R
			skillsMenuButton.gotoAndStop(34); //G
			changeTitleButton.gotoAndStop(34); //G
			showDetailButton.gotoAndStop(33); //F
		}else{
			CategoryTitle.moveLeft.gotoAndStop(274); //LB
			CategoryTitle.moveRight.gotoAndStop(275); //RB
			skillsMenuButton.gotoAndStop(279); //Y
			changeTitleButton.gotoAndStop(279); //Y
			showDetailButton.gotoAndStop(273); //RS
		}
	}
	
	function SetPlatform(a_platform:Number, a_bPS3Switch:Boolean):Void
	{
		var isGamepad = _platform != 0;

		if (a_platform == 0)
		{
			_deleteControls = {keyCode:45};// X
			_defaultControls = {keyCode:20};// T
			_kinectControls = {keyCode:37};// K
			_acceptControls = {keyCode:28};// Enter
			_cancelControls = {keyCode:15};// Tab
		}
		else
		{
			_deleteControls = {keyCode:278};// 360_X
			_defaultControls = {keyCode:279};// 360_Y
			_kinectControls = {keyCode:275};// 360_RB
			_acceptControls = {keyCode:276};// 360_A
			_cancelControls = {keyCode:277};// 360_B
		}

		_acceptButton.addEventListener("click",this,"onAcceptMousePress");
		_cancelButton.addEventListener("click",this,"onCancelMousePress");

		_platform = a_platform;
	}
	
	function alphabetSort(arr:Array, prop:String):Array {
		arr.sort(function(a, b) {
			var valA:String = String(a[prop]);
			var valB:String = String(b[prop]);
	
			// Remove leading "$" if present
			if (valA.charAt(0) == "$") {
				valA = valA.substr(1);
			}
			if (valB.charAt(0) == "$") {
				valB = valB.substr(1);
			}
	
			// Compare case-insensitively
			valA = valA.toLowerCase();
			valB = valB.toLowerCase();
	
			if (valA < valB) {
				return -1;
			} else if (valA > valB) {
				return 1;
			}
			return 0;
		});
		return arr;
	}
	
	function numericSort(arr:Array, prop:String, descending:Boolean):Array {
		arr.sort(function(a, b) {
			var valA:Number = Number(a[prop]);
			var valB:Number = Number(b[prop]);
	
			if (isNaN(valA)) valA = 0;
			if (isNaN(valB)) valB = 0;
	
			return descending ? (valB - valA) : (valA - valB);
		});
		return arr;
	}
	
	function SetWidescreen(widescreen:Boolean):Void {
		if(!widescreen){
			return;
		}
		
		BaseInfo_mc._x += 160;
		MenuLeftSide_mc._x -= 110;
	}
	
	function debugLog(message:Object):Void
	{
		if (typeof (message) == "string")
		{
			debugTextField.text += message + "\n";
		}
		else if (typeof (message) == "object")
		{
			var logText:String = "Object Details: ";
			for (var prop in message)
			{
				logText += prop + ": " + message[prop] + "; ";
			}
			debugTextField.text += logText + "\n";
		}
	}
	
	function debugLogHandler(event:Object):Void
	{
		debugLog(event.message);
	}
}