-- translation for Special 3v3 Package

return {
	["Special3v3"] = "3v3",
	["Special3v3Ext"] = "3v3扩展",
	["New3v3Card"] = "3v3卡牌(2012)",

	["#zhugejin"] = "联盟的维系者",
	["zhugejin"] = "诸葛瑾",
	["illustrator:zhugejin"] = "LiuHeng",
	["hongyuan"] = "弘援",
	[":hongyuan"] = "摸牌阶段，你可以少摸一张牌：若如此做，其他己方角色各摸一张牌。",
	[":hongyuan_p"] = "摸牌阶段，你可以少摸一张牌：若如此做，令至多两名其他角色各摸一张牌。",
	["@hongyuan"] = "你可以发动“弘援”",
	["~hongyuan"] = "选择 0-2 名其他角色→点击确定",
	["huanshi"] = "缓释",
	[":huanshi"] = "每当己方角色的判定牌生效前，你可以打出一张牌代替之。",
	[":huanshi_p"] = "每当一名角色的判定牌生效前，你可以令该角色观看你的手牌，然后该角色选择你的一张牌，你打出此牌代替之。",
	["@huanshi-card"] = CommonTranslationTable["@askforretrial"],
	["~huanshi"] = "选择一张牌→点击确定",
	["huanshi:accept"] = "接受改判",
	["huanshi:reject"] = "拒绝改判",
	["mingzhe"] = "明哲",
	[":mingzhe"] = "你的回合外，每当你使用或打出一张红色牌时，或因弃置而失去一张红色牌后，你可以摸一张牌。",

	["New3v3_2013Card"] = "3v3卡牌(2013)",

	["vs_nos_xiahoudun"] = "夏侯惇3v3",
	["&vs_nos_xiahoudun"] = "夏侯惇",
	["vsganglie"] = "刚烈",
	[":vsganglie"] = "每当你受到伤害后，你可以选择一名对方角色并进行判定：若结果不为红桃，则该角色选择一项：弃置两张手牌，或受到1点伤害。",
	[":vsganglie_p"] = "每当你受到伤害后，你可以选择一名其他角色并进行判定：若结果不为♥，则该角色选择一项：弃置两张手牌，或受到1点伤害。",
	["vsganglie-invoke"] = "你可以发动“刚烈”<br> <b>操作提示</b>: 选择一名其他角色→点击确定<br/>",

	["vs_nos_guanyu"] = "关羽3v3",
	["&vs_nos_guanyu"] = "关羽",
	["zhongyi"] = "忠义",
        [":zhongyi"] = "限定技，出牌阶段，你可以将一张红色手牌置于武将牌上。己方角色使用的【杀】对目标角色造成伤害时，若你有“忠义”牌，此伤害+1。身份牌重置后，你将“忠义”牌置入弃牌堆。",
        [":zhongyi_p"] = "限定技，出牌阶段，你可以将一张红色手牌置于武将牌上。【杀】对目标角色造成伤害时，若你有“忠义”牌，你可以令此伤害+1。你的下个回合开始时，你将“忠义”牌置入弃牌堆。",
	["loyal"] = "忠义",
	["$ZhongyiAnimate"] = "image=image/animate/zhongyi.png",
	["#ZhongyiBuff"] = "%from 的“<font color=\"yellow\"><b>忠义</b></font>”效果被触发，伤害从 %arg 点增加至 %arg2 点",

	["vs_nos_zhaoyun"] = "赵云3v3",
	["&vs_nos_zhaoyun"] = "赵云",
	["jiuzhu"] = "救主",
	[":jiuzhu"] = "每当一名其他己方角色处于濒死状态时，若你的体力值大于1，你可以失去1点体力并弃置一张牌：若如此做，该角色回复1点体力。",
	[":jiuzhu_p"] = "每当一名其他角色处于濒死状态时，若你的体力值大于1，你可以失去1点体力并弃置一张牌：若如此做，该角色回复1点体力。",
	["@jiuzhu"] = "你可以发动“救主”",

	["vs_nos_lvbu"] = "吕布3v3",
	["&vs_nos_lvbu"] = "吕布",
	["zhanshen"] = "战神",
        [":zhanshen"] = "觉醒技，准备阶段开始时，若你已受伤且有己方角色已死亡，你失去1点体力上限，弃置装备区的武器牌，然后获得“马术”和“神戟”。",
        [":zhanshen_p"] = "一名其他角色死亡时，可以令你获得一枚“战”标记。觉醒技，准备阶段开始时，若你已受伤且拥有“战”标记，你失去1点体力上限，弃置装备区的武器牌，然后获得“马术”和“神戟”。",
	["zhanshen:mark"] = "你可以令 %src 获得一枚“战”标记",
	["@fight"] = "战",
	["$ZhanshenAnimate"] = "image=image/animate/zhanshen.png",
	["#ZhanshenWake"] = "%from 已受伤且有己方角色已死亡，触发 %arg 觉醒",

	["#wenpin"] = "坚城宿将",
	["wenpin"] = "文聘",
	["illustrator:wenpin"] = "木美人",
	["zhenwei"] = "镇卫",
	["#zhenwei"] = "镇卫",
        [":zhenwei"] = "锁定技，对方角色与其他己方角色的距离+1。",
	[":zhenwei_p"] = "回合结束时，你可以令至多X名其他角色获得一枚“守”标记，直到你的下个回合结束时。除你和拥有“守”标记的角色与拥有“守”标记的角色距离+1。（X为本场游戏人数的一半减1，向下取整）",
	["@defense"] = "守",
	["@zhenwei"] = "你可以发动“镇卫”",
	["~zhenwei"] = "选择至多X名其他角色→点击确定",

	["vscrossbow"] = "连弩",
        [":vscrossbow"] = "装备牌·武器<br /><b>攻击范围</b>：１<br /><b>武器技能</b>：锁定技，出牌阶段，你可以额外使用三张【杀】。",
}
