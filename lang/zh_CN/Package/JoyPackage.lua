-- translation for JoyPackage

return {
	["joy"] = "欢乐包",

	["shit"] = "屎",
	[":shit"] = "基本牌\
作用效果：当此牌在<font color='red'><b>你的回合</b></font>内从你的<font color='red'>手牌</font>进入<font color='red'>弃牌堆</font>时，你将受到自己对自己的1点伤害（黑桃为流失1点体力），其中方块为无属性伤害、梅花为雷电伤害、红桃为火焰伤害。造成伤害的牌为此牌。在你的回合内，你可多次食用。",
	["disgusting_card"] = "恶心牌",
	["$ShitLostHp"] = "%from 吃了 %card, 将流失1点体力",
	["$ShitDamage"] = "%from 吃了 %card, 将受到自己对自己的1点伤害",

-- disaster
	["Disaster"] = "天灾包",

	["deluge"] = "洪水",
	[":deluge"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【洪水】放置于你的判定区里，回合判定阶段进行判定：若判定结果为 A,K，从当前角色的牌随机取出和场上存活人数相等的数量置于桌前，从下家开始，每人选一张收为手牌，将【洪水】置入弃牌堆。若判定结果不为AK，将【洪水】移到当前角色下家的判定区里",

	["typhoon"] = "台风",
	[":typhoon"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【台风】放置于你的判定区里，回合判定阶段进行判定：若判定结果为方块2~9之间，与当前角色距离为1的角色弃置6张手牌，将【台风】置入弃牌堆。若判定结果不为方块2~9之间，将【台风】移动到当前角色下家的判定区里",

	["earthquake"] = "地震",
	[":earthquake"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【地震】放置于你的判定区里，回合判定阶段进行判定：若判定结果为梅花2~9之间，与当前角色距离为1以内的角色(无视+1马)弃置装备区里的所有牌，将【地震】置入弃牌堆。若判定结果不为梅花2~9之间，将【地震】移动到当前角色下家的判定区里",

	["volcano"] = "火山",
	[":volcano"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【火山】放置于你的判定区里，回合判定阶段进行判定：若判定结果为红桃2~9之间，当前角色受到2点火焰伤害，与当前角色距离为1的角色(无视+1马)受到1点火焰伤害，【火山】生效后即置入弃牌堆。若判定结果不为红桃2~9之间，将【火山】移动到当前角色下家的判定区里",

	["mudslide"] = "泥石流",
	[":mudslide"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：将【泥石流】放置于你的判定区里，回合判定阶段进行判定：若判定结果为黑桃或梅花A,K,4,7，从当前角色开始，每名角色依次按顺序弃置武器、防具、+1马、-1马，无装备者受到1点无属性伤害，当总共被弃置的装备达到4件或你上家结算完成时，【泥石流】停止结算并置入弃牌堆。若判定牌不为黑色AK47，将【泥石流】移动到下家的判定区里",

-- equips
	["JoyEquip"] = "欢乐装备包",

	["monkey"] = "猴子",
	[":monkey"] = "装备牌·坐骑\
坐骑效果：1、当场上有其他角色使用【桃】时，你可以弃置【猴子】，阻止【桃】的结算并将其收为手牌；2、你计算与其他角色的距离时，始终-1",
	["grab_peach"] = "偷桃",

	["gale_shell"] = "狂风甲",
	[":gale_shell"] = "装备牌·防具\
防具效果：1、锁定技，每次受到火焰伤害时，该伤害+1；2、你可以将狂风甲装备和你距离为1以内的一名角色的装备区内",
	["#GaleShellDamage"] = "%from 装备【狂风甲】的效果被触发，由 %arg 点火焰伤害增加到 %arg2 点火焰伤害",

	["yx_sword"] = "杨修剑",
	[":yx_sword"] = "装备牌·武器\
攻击范围：３<br/>武器特效：当你的【杀】造成伤害时，可以指定攻击范围内的一名其他角色为伤害来源，杨修剑归该角色所有",
	["@yxsword-select"] = "你可以选择一名你攻击范围内的其他角色为此伤害的来源",

	["five_lines"] = "五道杠",
	[":five_lines"] = "装备牌·防具\
防具效果：当你的体力值：\
不小于5，视为你拥有“苦肉”；\
为4，视为你拥有“国色”；\
为3，视为你拥有“结姻”；\
为2，视为你拥有“集智”；\
不大于1，视为你拥有“仁德”",
}
