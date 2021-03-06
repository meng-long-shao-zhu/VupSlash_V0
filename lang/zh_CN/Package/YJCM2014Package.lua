-- translation for YJCM2014 Package

return {
	["YJCM2014"] = "一将成名2014",

	["#caifuren"] = "襄江的蒲苇",
	["caifuren"] = "蔡夫人",
	["illustrator:caifuren"] = "Dream彼端",
	["designer:caifuren"] = "B.LEE",
	["qieting"] = "窃听",
	[":qieting"] = "一名其他角色的回合结束时，若其未于此回合内使用过以除其外的角色为目标的牌，你可以选择一项：将其装备区的一张牌置入自己的装备区，或摸一张牌。",
	["qieting:0"] = "移动武器牌",
	["qieting:1"] = "移动防具牌",
	["qieting:2"] = "移动+1坐骑",
	["qieting:3"] = "移动-1坐骑",
	["qieting:4"] = "移动宝物牌",
	["qieting:draw"] = "摸一张牌",
	["xianzhou"] = "献州",
	[":xianzhou"] = "限定技，出牌阶段，你可以将装备区的所有牌交给一名其他角色，然后该角色选择一项：令你回复X点体力，或对其攻击范围内的X名角色各造成1点伤害。（X为你以此法给出的牌数）",
	["@xianzhou-damage"] = "请对攻击范围内的 %arg 名角色各造成 1 点伤害",
	["~xianzhou"] = "选择若干名角色→点击确定",
	["$XianzhouAnimate"] = "image=image/animate/xianzhou.png",

	["#caozhen"] = "荷国天督",
	["caozhen"] = "曹真",
	["illustrator:caozhen"] = "Thinking",
	["designer:caozhen"] = "世外高v狼",
	["sidi"] = "司敌",
	[":sidi"] = "每当你使用或其他角色在你的回合内使用【闪】时，你可以将牌堆顶的一张牌置于武将牌上。一名其他角色的出牌阶段开始时，你可以将一张“司敌牌”置入弃牌堆，然后该角色本回合使用【杀】的次数上限-1。",
	["sidi_remove"] = "你可以将一张“司敌牌”置入弃牌堆，令当前回合角色本回合使用【杀】的次数上限-1",
	["~sidi"] = "选择一张“司敌牌”→点击确定",
	
	["new_caozhen"] = "新曹真",
	["&new_caozhen"] = "曹真",
	["newsidi"] = "司敌",
	[":newsidi"] = "其他角色出牌阶段开始时，你可以弃置一张非基本牌（须与你装备区里的牌颜色相同），然后该角色本回合不能使用和打出与此牌颜色相同的牌。此阶段结束时，若其于此阶段内没有使用【杀】，视为你对其使用一张【杀】。",
	["@newsidi-discard"] = "你可以弃置一张牌对 %src 发动“司敌”",

	["#chenqun"] = "万世臣表",
	["chenqun"] = "陈群",
	["illustrator:chenqun"] = "DH",
	["designer:chenqun"] = "To Joanna",
	["dingpin"] = "定品",
	[":dingpin"] = "出牌阶段，你可以弃置一张与你本回合已使用或弃置的牌类别均不同的手牌，然后令一名已受伤的角色进行判定：若结果为黑色，该角色摸X张牌，且你本阶段不能对该角色发动“定品”；红色，你将武将牌翻面。（X为该角色已损失的体力值）",
	["faen"] = "法恩",
	[":faen"] = "每当一名角色的武将牌翻面或横置时，你可以令其摸一张牌。",
	
	["new_chenqun"] = "新陈群",
	["&new_chenqun"] = "陈群",
	["newdingpin"] = "定品",
	[":newdingpin"] = "出牌阶段，你可以弃置一张牌并选择一名其他角色（不能弃置相同类型牌且不能指定相同的角色），然后令其执行一项：摸X张牌；弃置X张牌（X为本回合此技能发动次数）。若其已受伤，你将武将牌横置。",
	["newdingpin:draw"] = "令其摸牌",
	["newdingpin:discard"] = "令其弃牌",
	["newfaen"] = "法恩",
	[":newfaen"] = "当一名角色的武将牌翻至正面向上或横置后，你可以令其摸一张牌。",
	
	["#guyong"] = "庙堂的玉磬",
	["guyong"] = "顾雍",
	["illustrator:guyong"] = "大佬荣",
	["designer:guyong"] = "睿笛终落",
	["shenxing"] = "慎行",
	[":shenxing"] = "出牌阶段，你可以弃置两张牌：若如此做，你摸一张牌。",
	["bingyi"] = "秉壹",
	[":bingyi"] = "结束阶段开始时，若你有手牌，你可以展示所有手牌：若均为同一颜色，你可以令至多X名角色各摸一张牌。（X为你的手牌数）",
	["@bingyi-card"] = "你可以展示所有手牌发动“秉壹”",
	["~bingyi"] = "若手牌均为同一颜色，选择至多X名角色→点击确定；否则直接点击确定",

	["#hanhaoshihuan"] = "中军之主",
	["hanhaoshihuan"] = "韩浩＆史涣",
	["&hanhaoshihuan"] = "韩浩史涣",
	["illustrator:hanhaoshihuan"] = "L",
	["designer:hanhaoshihuan"] = "浪人兵法家",
	["shenduan"] = "慎断",
	[":shenduan"] = "每当你的黑色基本牌因弃置而置入弃牌堆时，你可以将之当【兵粮寸断】使用（无距离限制）。",
	["@shenduan-use"] = "你可以发动“慎断”将其中一张牌当【兵粮寸断】使用（无距离限制）",
	["~shenduan"] = "选择一张黑色基本牌→选择【兵粮寸断】的目标角色→点击确定",
	["yonglve"] = "勇略",
	[":yonglve"] = "每当你攻击范围内的一名角色的判定阶段开始时，你可以弃置其判定区的一张牌：若如此做，视为对该角色使用一张【杀】（无距离限制）：若此【杀】未造成伤害，此【杀】结算后你摸一张牌。",

	["#jvshou"] = "监军谋国",
	["jvshou"] = "沮授",
	["illustrator:jvshou"] = "酱油之神",
	["designer:jvshou"] = "精精神神",
	["jianying"] = "渐营",
	[":jianying"] = "每当你于出牌阶段内使用一张牌时，若此牌与你本阶段使用的上一张牌花色或点数相同，你可以摸一张牌。",
	["shibei"] = "矢北",
    [":shibei"] = "锁定技，每当你于一名角色的回合内受到伤害后，若为你本回合第一次受到伤害，你回复1点体力，否则你失去1点体力。",

	["#sunluban"] = "为虎作伥",
	["sunluban"] = "孙鲁班",
	["illustrator:sunluban"] = "FOOLTOWN",
	["designer:sunluban"] = "CatCat44",
	["zenhui"] = "谮毁",
    [":zenhui"] = "出牌阶段限一次，每当你于出牌阶段内使用【杀】或黑色非延时类锦囊牌指定唯一目标时，你可以令为此牌合法目标的另一名其他角色选择一项：交给你一张牌并成为此牌的使用者，或成为此牌的目标。",
	["zenhui-invoke"] = "你可以发动“谮毁”<br/> <b>操作提示</b>: 选择除 %src 外的一名角色→点击确定<br/>",
	["@zenhui-collateral"] = "请选择【借刀杀人】 %src 使用【杀】的目标",
	["@zenhui-give"] = "请交给 %src 一张牌成为此牌的使用者，否则你成为此牌的目标",
	["jiaojin"] = "骄矜",
	[":jiaojin"] = "每当你受到男性角色的伤害时，你可以弃置一张装备牌：若如此做，此伤害-1。",
	["@jiaojin"] = "你可以弃置一张装备牌发动“骄矜”令此伤害-1",
	["#Jiaojin"] = "%from 发动了“<font color=\"yellow\"><b>骄矜</b></font>”，伤害从 %arg 点减少至 %arg2 点",

	["#wuyi"] = "建兴鞍辔",
	["wuyi"] = "吴懿",
	["illustrator:wuyi"] = "蚂蚁君",
	["desinger:wuyi"] = "沸治克里夫",
	["benxi"] = "奔袭",
    [":benxi"] = "锁定技，你的回合内，你与其他角色的距离-X。你的回合内，若你与所有其他角色距离均为1，其他角色的防具无效，你使用【杀】可以额外选择一个目标。（X为本回合你已使用结算完毕的牌数）",
	["#benxi-dist"] = "奔袭",

	["#zhangsong"] = "怀璧待凤仪",
	["zhangsong"] = "张松",
	["illustrator:zhangsong"] = "尼乐小丑",
	["designer:zhangsong"] = "铁血冥剑",
	["qiangzhi"] = "强识",
	[":qiangzhi"] = "出牌阶段开始时，你可以展示一名其他角色的一张手牌：若如此做，每当你于此阶段内使用与此牌类别相同的牌时，你可以摸一张牌。",
	["qiangzhi-invoke"] = "你可以发动“强识”<br/> <b>操作提示</b>: 选择一名有手牌的其他角色→点击确定<br/>",
	["xiantu"] = "献图",
	[":xiantu"] = "一名其他角色的出牌阶段开始时，你可以摸两张牌：若如此做，你交给其两张牌；且本阶段结束后，若该角色未于本阶段杀死过一名角色，你失去1点体力。",
	["@xiantu-give"] = "请交给 %dest %arg 张牌",
	["#Xiantu"] = "%from 未于本阶段杀死过角色，%to 的“%arg”被触发",

	["#zhoucang"] = "披肝沥胆",
	["zhoucang"] = "周仓",
	["illustrator:zhoucang"] = "Sky",
	["designer:zhoucang"] = "WOLVES29",
	["zhongyong"] = "忠勇",
	[":zhongyong"] = "每当你于出牌阶段内使用【杀】被【闪】抵消后，你可以令除目标角色外的一名角色获得弃牌堆中的此次使用的【闪】：若该角色不为你，你可以对同一目标角色再使用一张【杀】（无距离限制且不能选择额外目标）。",
	["zhongyong-invoke"] = "你可以发动“忠勇”<br/> <b>操作提示</b>: 选择除 %src 外的一名角色→点击确定<br/>",
	["zhongyong-slash"] = "你可以发动“忠勇”再对 %src 使用一张【杀】",
	
	["new_zhoucang"] = "新周仓",
	["&new_zhoucang"] = "周仓",
	["newzhongyong"] = "忠勇",
	[":newzhongyong"] = "当你使用的【杀】结算完后，你可以将此【杀】或目标角色使用的【闪】交给除目标角色外的一名其他角色，若其获得了红色牌，其可以对你攻击范围内的角色使用一张【杀】。",
	["newzhongyong:slash"] = "令其获得【杀】",
	["newzhongyong:jink"] = "令其获得【闪】",
	["@newzhongyong-invoke"] = "你可以发动“忠勇”",
	["@newzhongyong-slash"] = "你可以使用一张【杀】",
	
	["#zhuhuan"] = "中洲拒天人",
	["zhuhuan"] = "朱桓",
	["illustrator:zhuhuan"] = "XXX",
	["designer:zhuhuan"] = "半缘修道",
	["youdi"] = "诱敌",
	[":youdi"] = "结束阶段开始时，你可以令一名其他角色弃置你一张牌：若此牌不为【杀】，你获得其一张牌。",
	["youdi-invoke"] = "你可以发动“诱敌”<br> <b>操作提示</b>：选择一名其他角色→点击确定<br/>",
	["youdi_obtain"] = "诱敌获得牌",
	
	["new_zhuhuan"] = "新朱桓",
	["&new_zhuhuan"] = "朱桓",
	["fenli"] = "奋励",
	[":fenli"] = "若你的手牌数为全场最多，你可以跳过摸牌阶段；若你的体力值为全场最多，你可以跳过出牌阶段；若你的装备区里有牌且数量为全场最多，你可以跳过弃牌阶段。",
	["fenli:draw"] = "你是否发动“奋励”跳过摸牌阶段？",
	["fenli:play"] = "你是否发动“奋励”跳过出牌阶段？",
	["fenli:discard"] = "你是否发动“奋励”跳过弃牌阶段？",
	["pingkou"] = "平寇",
	[":pingkou"] = "回合结束时，你可以对至多X名其他角色各造成1点伤害（X为你本回合跳过的阶段数）。",
	["@pingkou"] = "你可以对至多 %src 名角色各造成1点伤害",
	["~pingkou"] = "选择若干名目标角色→点“确定”",
}
