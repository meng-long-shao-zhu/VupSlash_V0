-- translation for StandardPackage

local t = {
	["standard_cards"] = "标准版",

	["slash"] = "杀",
	[":slash"] = "基本牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：攻击范围内的一名其他角色<br /><b>效果</b>：对目标角色造成1点伤害。",
	["slash-jink"] = "%src 使用了【杀】，请使用一张【闪】",
	["@multi-jink-start"] = "%src 使用了【杀】，你须连续使用 %arg 张【闪】",
	["@multi-jink"] = "%src 使用了【杀】，你须再使用 %arg 张【闪】",
	["@slash_extra_targets"] = "请选择此【杀】的额外目标",

	["jink"] = "闪",
	[":jink"] = "基本牌<br /><b>时机</b>：【杀】对你生效时<br /><b>目标</b>：此【杀】<br /><b>效果</b>：抵消此【杀】的效果。",
	["#NoJink"] = "%from 不能使用【<font color=\"yellow\"><b>闪</b></font>】响应此【<font color=\"yellow\"><b>杀</b></font>】",

	["peach"] = "桃",
	[":peach"] = "基本牌<br /><b>时机</b>：出牌阶段/当一名角色处于濒死状态时<br /><b>目标</b>：已受伤的你/一名处于濒死状态的角色<br /><b>效果</b>：目标角色回复1点体力。",

	["crossbow"] = "诸葛连弩",
	[":crossbow"] = "装备牌·武器<br /><b>攻击范围</b>：１<br /><b>武器技能</b>：你使用【杀】无次数限制。",

	["double_sword"] = "雌雄双股剑",
	[":double_sword"] = "装备牌·武器<br /><b>攻击范围</b>：２<br /><b>武器技能</b>：当你使用【杀】指定一个异性目标后，你可以令其选择一项：1．弃置一张手牌；2．令你摸一张牌。",
	["double-sword-card"] = "%src 发动了【雌雄双股剑】效果，你须弃置一张手牌，或令 %src 摸一张牌",

	["qinggang_sword"] = "青釭剑",
	[":qinggang_sword"] = "装备牌·武器<br /><b>攻击范围</b>：２<br /><b>武器技能</b>：锁定技，当你使用【杀】指定一个目标后，你无视其防具。",

	["blade"] = "青龙偃月刀",
	[":blade"] = "装备牌·武器<br /><b>攻击范围</b>：３<br /><b>武器技能</b>：当你使用的【杀】被目标角色使用的【闪】抵消时，你可以对其使用【杀】。（无距离限制）",
	["blade-slash"] = "你可以发动【青龙偃月刀】再对 %src 使用一张【杀】",
	["#BladeUse"] = "%from 对 %to 发动了【<font color=\"yellow\"><b>青龙偃月刀</b></font>】效果",

	["spear"] = "丈八蛇矛",
	[":spear"] = "装备牌·武器<br /><b>攻击范围</b>：３<br /><b>武器技能</b>：你可以将两张手牌当【杀】使用或打出。",

	["axe"] = "贯石斧",
	[":axe"] = "装备牌·武器<br /><b>攻击范围</b>：３<br /><b>武器技能</b>：当你使用的【杀】被【闪】抵消时，你可以弃置两张牌，令此【杀】依然生效。",
	["@axe"] = "你可以弃置两张牌令此【杀】继续造成伤害",
	["~axe"] = "选择两张牌→点击确定",

	["halberd"] = "方天画戟",
	[":halberd"] = "装备牌·武器<br /><b>攻击范围</b>：４<br /><b>武器技能</b>：你使用是最后的手牌的【杀】的额外目标数上限+2。",

	["kylin_bow"] = "麒麟弓",
	[":kylin_bow"] = "装备牌·武器<br /><b>攻击范围</b>：５<br /><b>武器技能</b>：当你使用【杀】对目标角色造成伤害时，你可以破坏其装备区里的一张坐骑牌。",
	["kylin_bow:dhorse"] = "+1坐骑",
	["kylin_bow:ohorse"] = "-1坐骑",

	["eight_diagram"] = "八卦阵",
	[":eight_diagram"] = "装备牌·防具<br /><b>防具技能</b>：当你需要使用／打出【闪】时，你可以判定，若结果为红色，你视为使用／打出【闪】。",

	["standard_ex_cards"] = "标准版EX",

	["renwang_shield"] = "仁王盾",
	[":renwang_shield"] = "装备牌·防具<br /><b>防具技能</b>：锁定技，黑色【杀】对你无效。",

	["ice_sword"] = "寒冰剑",
	[":ice_sword"] = "装备牌·武器<br /><b>攻击范围</b>：２<br /><b>武器技能</b>：当你使用【杀】对目标角色造成伤害时，若其有牌，你可以防止此伤害，依次破坏其两张牌。",
	["ice_sword:yes"] = "你可以依次破坏其两张牌",

	["horse"] = "坐骑",
	[":+1 horse"] = "装备牌·坐骑<br /><b>坐骑技能</b>：其他角色与你的距离+1。",
	["jueying"] = "绝影",
	["dilu"] = "的卢",
	["zhuahuangfeidian"] = "爪黄飞电",
	[":-1 horse"] = "装备牌·坐骑<br /><b>坐骑技能</b>：你与其他角色的距离-1。",
	["chitu"] = "赤兔",
	["dayuan"] = "大宛",
	["zixing"] = "紫骍",

	["amazing_grace"] = "五谷丰登",
	[":amazing_grace"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：所有角色<br /><b>执行动作</b>：当此牌指定目标后，你亮出牌堆顶的X张牌。（X 为目标数）<br /><b>效果</b>：目标角色选择一张其中未被获得的牌并获得之。",

	["god_salvation"] = "桃园结义",
	[":god_salvation"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：所有角色<br /><b>效果</b>：目标角色回复1点体力。",

	["savage_assault"] = "南蛮入侵",
	[":savage_assault"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：所有其他角色<br /><b>效果</b>：目标角色需打出【杀】，否则受到1点伤害。",
	["savage-assault-slash"] = "%src 使用了【南蛮入侵】，请打出一张【杀】来响应",

	["archery_attack"] = "万箭齐发",
	[":archery_attack"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：所有其他角色<br /><b>效果</b>：目标角色需打出【闪】，否则受到1点伤害",
	["archery-attack-jink"] = "%src 使用了【万箭齐发】，请打出一张【闪】以响应",

	["collateral"] = "借刀杀人",
	[":collateral"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>一名装备区里有武器牌且攻击范围内有其使用【杀】的合法目标的其他角色。<br /><b>执行动作</b>：选择目标攻击范围内的一个合法目标。<br /><b>效果</b>：目标需对你为其选择的合法目标使用【杀】，否则你获得其装备区内的武器牌。",
	["collateral-slash"] = "%dest 使用了【借刀杀人】，请对 %src 使用一张【杀】",
	["#CollateralSlash"] = "%from 选择了此【<font color=\"yellow\"><b>杀</b></font>】的目标 %to",

	["duel"] = "决斗",
	[":duel"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：一名其他角色<br /><b>效果</b>：由目标角色开始，其与你轮流打出【杀】，直到其中的一名角色未打出【杀】。然后未打出【杀】的角色受到另一名角色造成的1点伤害。",
	["duel-slash"] = "%src 对你【决斗】，你需要打出一张【杀】",

	["ex_nihilo"] = "无中生有",
	[":ex_nihilo"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：你<br /><b>效果</b>：目标角色摸两张牌。",

	["snatch"] = "顺手牵羊",
	[":snatch"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：距离为1 的一名区域里有牌的其他角色<br /><b>效果</b>：获得目标角色的区域里的一张牌。",

	["dismantlement"] = "过河拆桥",
	[":dismantlement"] = "锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：一名区域内里牌的其他角色。<br /><b>效果</b>：破坏目标角色区域里的一张牌。",

	["nullification"] = "无懈可击",
	[":nullification"] = "锦囊牌<br /><b>时机</b>：锦囊牌对一个目标生效前<br /><b>目标</b>：此锦囊牌<br /><b>效果</b>：抵消此锦囊牌的效果。",

	["indulgence"] = "乐不思蜀",
	[":indulgence"] = "延时锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：一名其他角色<br /><b>效果</b>：目标角色判定，若结果不为红桃，其跳过出牌阶段。",

	["lightning"] = "闪电",
	[":lightning"] = "延时锦囊牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：你<br /><b>效果</b>：目标角色判定，若结果为黑桃2～9，其受到3点雷电伤害，将此【闪电】置入弃牌堆，否则将之置于下家的判定区。",

	["limitation_broken"] = "界限突破卡牌",

	["wooden_ox"] = "木牛流马",
	[":wooden_ox"] = "装备牌·宝物<br /><b>宝物技能</b>：<br />出牌阶段限一次，你可以将一张手牌移出游戏并扣置于此牌下，若如此做，你可以将装备区里的此牌置入一名其他角色的装备区；你可以将此装备牌下的牌如手牌般使用或打出。",
	["@wooden_ox-move"] = "你可以将【木牛流马】移动至一名其他角色的装备区",
	["#WoodenOx"] = "%from 使用/打出了 %arg 张 %arg2 牌",
}

local ohorses = { "chitu", "dayuan", "zixing" }
local dhorses = { "zhuahuangfeidian", "dilu", "jueying", "hualiu" }

for _, horse in ipairs(ohorses) do
	t[":" .. horse] = t[":-1 horse"]
end

for _, horse in ipairs(dhorses) do
	t[":" .. horse] = t[":+1 horse"]
end

return t