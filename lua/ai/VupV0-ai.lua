
--------------------------------------------------
--判断是否能被技能指定为目标
--注意与VupV0.lua同步
--------------------------------------------------

function SkillCanTarget(to, player, skill_name)	--判断是否能被技能指定为目标
	if player:objectName() ~= to:objectName() and to:hasSkill("muying_wucailongguo_lianggonglin_vup") then	--幕影
		return false
	end
	return true
end

--------------------------------------------------
--函数部分
--------------------------------------------------

--根据flag找角色--
function findPlayerByFlag(room, flag_name)
	for _,p in sgs.qlist(room:getAllPlayers(true)) do
		if p:hasFlag(flag_name) then
			return p
		end
	end
	return nil
end

--根据objectName找角色--
function findPlayerByObjName(room, obj_name)
	for _,p in sgs.qlist(room:getAllPlayers(true)) do
		if p:objectName() == obj_name then
			return p
		end
	end
	return nil
end

--缔盟的弃牌策略--
local dimeng_discard = function(self, discard_num, mycards)
	local cards = mycards
	local to_discard = {}

	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2
			elseif card:isKindOf("OffensiveHorse") then return 1
			elseif card:isKindOf("Weapon") then return 2
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then return 4
			end
		elseif self:getUseValue(card) >= 6 then return 3 --使用价值高的牌，如顺手牵羊(9),下调至桃
		elseif self:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0
		end
		return 0
	end

	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then
			return aux_func(a) < aux_func(b)
		end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if not self.player:isJilei(card) then table.insert(to_discard, card:getId()) end
		if #to_discard >= discard_num then break end
	end
	if #to_discard ~= discard_num then return {} end
	return to_discard
end

--------------------------------------------------

--------------------------------------------------
--残机（两轮车）
--------------------------------------------------

local lianglunche_vs_skill={}
lianglunche_vs_skill.name="lianglunche_vs"
table.insert(sgs.ai_skills,lianglunche_vs_skill)
lianglunche_vs_skill.getTurnUseCard=function(self)
	if self.player:getMark("lianglunche_notready") > 0 or self.player:getMark("last_used_id_in_play_phase") == 0 or self.player:getPhase() ~= sgs.Player_Play or self.player:getMark("Equips_Nullified_to_Yourself") > 0 then return end
	local name = sgs.Sanguosha:getCard(self.player:getMark("last_used_id_in_play_phase")-1):objectName()
	if not name or name == "" then return end
	
	local cards = self.player:getCards("e")
	cards = sgs.QList2Table(cards)

	local card

	for _,acard in ipairs(cards) do
		if acard:isKindOf("Lianglunche") then
			card = acard
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = (""..name..":lianglunche_vs[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)

	return skillcard
end

--------------------------------------------------
--应援
--------------------------------------------------

local HLyingyuan_skill={}
HLyingyuan_skill.name="HLyingyuan"
table.insert(sgs.ai_skills,HLyingyuan_skill)
HLyingyuan_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("#HLyingyuanCard") and not self.player:isKongcheng() and not self:needBear() then
		return sgs.Card_Parse("#HLyingyuanCard:.:")
	end
end

sgs.ai_skill_use_func["#HLyingyuanCard"] = function(card, use, self)
	self:sort(self.friends_noself, "defense")
	local hasfriend = false
	for _, friend in ipairs(self.friends_noself) do
		if friend:isAlive() and not hasManjuanEffect(friend) and not (self:needKongcheng(friend) and friend:isKongcheng()) and (self:getOverflow(self.player, true) > 0 or ((self:isWeak(friend) or (friend:getHp() + friend:getHandcardNum() <= self.player:getHp() + self.player:getHandcardNum()) and self.player:getHandcardNum() >= 2) and not self:isWeak())) then
			if self.player:getRole() == friend:getRole() or (self.player:getRole() == "lord" and friend:getRole() == "loyalist") or (self.player:getRole() == "loyalist" and friend:getRole() == "lord") then
				hasfriend = true
				target = friend
				break
			end
		end
	end
	
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local compare_func = function(a, b)
		local v1 = self:getKeepValue(a)
		local v2 = self:getKeepValue(b)
		return v1 > v2
	end
	table.sort(cards, compare_func)
	
	
	if not hasfriend then return end
	for _, scard in ipairs(cards) do
		if scard then
			local card_str = "#HLyingyuanCard:"..scard:getId()..":->"..target:objectName()
			local acard = sgs.Card_Parse(card_str)
			assert(acard)
			use.card = acard
			break
		end
	end
	if use.to then
		use.to:append(target)
	end
	return
end

sgs.ai_use_priority["HLyingyuanCard"] = 1.3





--嘲讽
sgs.ai_chaofeng.baishenyao_zhaijiahaibao = 1

--------------------------------------------------
--抽卡
--------------------------------------------------

sgs.ai_skill_invoke.chouka = function(self, data)
	return true
end

sgs.ai_skill_choice.chouka = function(self, choices)
	if self.player:getHandcardNum() <= 1 or self.player:getMark("chouka_AI") > 1 then
		return "chouka_stop"
	end
	return "chouka_repeat"
end

--------------------------------------------------
--慵懒
--------------------------------------------------

sgs.ai_view_as.yonglan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if player:getMark("yonglan_number") ~= 0 and card:getNumber() > player:getMark("yonglan_number") and card_place == sgs.Player_PlaceHand then
		return ("jink:yonglan[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.ai_cardneed.yonglan = function(to, card, self)	--需要大点至少一张
	local cards = to:getHandcards()
	local has_big = false
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", self.room:getCurrent():objectName(), to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:getNumber() > 10 then
				has_big = true
				break
			end
		end
	end
	if not has_big then
		return card:getNumber() > 10
	end
end



--嘲讽
sgs.ai_chaofeng.hongxiaoyin_qiannianmofashi = -2

--------------------------------------------------
--永宁
--------------------------------------------------

sgs.ai_skill_playerchosen.yongning = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	self:sort(targetlist, "handcard")
	
	local first, second, third
	
	for _, p in ipairs(targetlist) do
		if self:isEnemy(p) then
			if not first and (p:hasSkills("xianwei|diyin|yunyao") and p:getMark("&yongning+TrickCard") == 0) then	--优先特化针对
				first = p
			elseif not second and p:getMark("&yongning+BasicCard") == 0 then
				second = p
			elseif not third then
				third = p
			end
		end
	end
	return first or second or third
end

sgs.ai_playerchosen_intention.yongning = function(self, from, to)
	local intention = 10
	sgs.updateIntention(from, to, intention)
end

sgs.ai_skill_choice.yongning = function(self, choices)
	local items = choices:split("+")
    if #items == 1 then
        return items[1]
	else
		local to = findPlayerByFlag(self.room, "yongning_target_AI")
		if to then
			if to:hasSkills("xianwei|diyin|yunyao") and table.contains(items, "TrickCard") then	--特化封锦囊
				return "TrickCard"
			end
		end
		if table.contains(items, "BasicCard") then
			return "BasicCard"
		elseif table.contains(items, "TrickCard") then
			return "TrickCard"
		elseif table.contains(items, "EquipCard") then
			return "EquipCard"
		end
	end
end




--嘲讽
sgs.ai_chaofeng.qiulinzi_wangyinwunv = 2

--------------------------------------------------
--礼崩
--------------------------------------------------

sgs.ai_skill_invoke.libeng = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		return false
	end
	return true
end

sgs.ai_cardneed.libeng = function(to, card, self)
	return card:isKindOf("Slash") or card:isKindOf("GudingBlade")
end

--------------------------------------------------
--超度
--------------------------------------------------

sgs.ai_skill_choice.chaodu = function(self, choices)
	local items = choices:split("+")
    if #items == 1 then
        return items[1]
	else
		local dying = findPlayerByFlag(self.room, "chaodu_target_AI")
		if table.contains(items, "chaodu_counter") and dying and dying:hasSkills(sgs.exclusive_skill) and self:isEnemy(dying) then
			return "chaodu_counter"
		elseif table.contains(items, "chaodu_recover") and self:isWeak() and self.player:isWounded() then
			return "chaodu_recover"
		elseif table.contains(items, "chaodu_draw") then
			return "chaodu_draw"
		end
	end
	return "cancel"
end




--嘲讽
sgs.ai_chaofeng.lingnainainai_fentujk = 2

--------------------------------------------------
--嗜甜
--------------------------------------------------

sgs.ai_skill_invoke.shitian = function(self, data)
	return true
end

--------------------------------------------------
--藏聪
--------------------------------------------------

sgs.ai_skill_invoke.cangcong = function(self, data)
	local objname = string.sub(data:toString(), 8, -1)	--截取choice:后面的部分(即player:objectName())
	if objname == "analeptic" or objname == "peach" or objname == "amazing_grace" or objname == "ex_nihilo" or objname == "god_salvation" then
		return false
	end
	local from = findPlayerByFlag(self.room, "cangcong_usefrom_AI")
	if (objname == "iron_chain" or objname == "fudichouxin" or objname == "snatch" or objname == "dismantlement") and from and self:isFriend(from) then
		return false
	end
	if (objname == "slash" or objname == "fire_slash" or objname == "thunder_slash" or objname == "archery_attack" or objname == "savage_assault" or objname == "fire_attack") and self:needToLoseHp() then
		return false
	end
	if (objname == "fire_slash" or objname == "thunder_slash" or objname == "fire_attack") and from and self:isFriend(from) then
		return false
	end
	return true
end




--嘲讽
sgs.ai_chaofeng.buding_qiaoxinmiyou = 3

--------------------------------------------------
--天巧
--------------------------------------------------

tianqiao_skill = {}
tianqiao_skill.name = "tianqiao"
table.insert(sgs.ai_skills, tianqiao_skill)
tianqiao_skill.getTurnUseCard = function(self)
	if self.player:getPhase() == sgs.Player_Play and self.player:getMark("tianqiao_triggering") > 0 and self.player:getMark("tianqiao_used") == 0 then
		return sgs.Sanguosha:getCard(self.room:getDrawPile():first())
	end
end

--------------------------------------------------
--宴灵
--------------------------------------------------

sgs.ai_skill_invoke.yanling = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		if getCardsNum("Peach", self.player, self.player) > 0 then	--宴灵已被算入桃数
			return true
		end
	end
	return false
end




--嘲讽
sgs.ai_chaofeng.yizhiyy_mianbaoren = 0

--------------------------------------------------
--卓识
--------------------------------------------------

sgs.ai_skill_invoke.zhuoshi = function(self, data)
	return true
end

sgs.ai_skill_playerchosen.zhuoshi = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	self:sort(targetlist, "defense")
	for _, p in ipairs(targetlist) do
		if self:isFriend(p) and not hasManjuanEffect(p) then
			return p
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.zhuoshi = function(self, from, to)
	local intention = -10
	if (to:hasSkill("kongcheng") and to:isKongcheng()) or hasManjuanEffect(to) then
		intention = 0
	end
	sgs.updateIntention(from, to, intention)
end

sgs.ai_cardneed.zhuoshi = function(to, card, self)
	return card:isKindOf("TrickCard")
end

--------------------------------------------------
--法棍
--------------------------------------------------

sgs.ai_cardneed.fagun = function(to, card, self)
	return card:isKindOf("BasicCard") and card:isRed()
end




--嘲讽
sgs.ai_chaofeng.liantai_bingyuanlangwang = 0

--------------------------------------------------
--逐猎
--------------------------------------------------

sgs.ai_cardneed.zhulie = function(to, card, self)
	return card:isKindOf("Slash")
end

--------------------------------------------------
--混音
--------------------------------------------------

sgs.ai_skill_invoke.hunyin = function(self, data)
	return true
end

sgs.ai_skill_invoke["hunyin_throw"] = function(self, data)
	local first = sgs.Sanguosha:getCard(self.room:getDrawPile():at(1))	--反向置于
	local second = sgs.Sanguosha:getCard(self.room:getDrawPile():at(0))	--反向置于
	if not self.player:getJudgingArea():isEmpty() then
		local last_judge = self.player:getJudgingArea():last()	--获取判定区最后一张牌（将第一个判定）
		if last_judge and last_judge:isKindOf("Indulgence") and first:getSuit() ~= sgs.Card_Heart then
			return true
		end
		if last_judge and last_judge:isKindOf("SupplyShortage") and first:getSuit() ~= sgs.Card_Club then
			return true
		end
		if last_judge and last_judge:isKindOf("Lightning") and first:getSuit() == sgs.Card_Spade and first:getNumber() >= 2 and first:getNumber() <= 9 then
			return true
		end
		return false
	else
		if (first:isKindOf("ExNihilo") or second:isKindOf("ExNihilo")) and not self:willSkipPlayPhase() then
			return false
		end
		local average_use_value = (self:getUseValue(first) + self:getUseValue(second)) / 2
		return average_use_value > 6
	end
	return false
end

sgs.ai_cardneed.hunyin = function(to, card, self)
	return card:isKindOf("Slash")
end




--嘲讽
sgs.ai_chaofeng.xingzhigumiya_mengmao = 0

--------------------------------------------------
--衔尾
--------------------------------------------------

local xianwei_skill={}
xianwei_skill.name="xianwei"
table.insert(sgs.ai_skills,xianwei_skill)
xianwei_skill.getTurnUseCard=function(self)
	if self.player:getMark("&xianwei_count") <= 2 and self.player:isWounded() and not self.player:isKongcheng() then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
	
		if self.player:getMark("&xianwei_count") == 0 then	--第一次
			local card
		
			self:sortByUseValue(cards,true)
		
			for _,acard in ipairs(cards) do
				if not acard:isKindOf("ExNihilo") and (self:getDynamicUsePriority(acard) <= 9 or self:getOverflow() > 0) then
					card = acard
				end
			end
		
			if not card then return nil end
			local card_str = ("ex_nihilo:xianwei[%s:%s]=%d"):format("to_be_decided", 0, card:getEffectiveId())
			local skillcard = sgs.Card_Parse(card_str)
		
			assert(skillcard)
		
			return skillcard
		elseif self.player:getMark("&xianwei_count") == 1 and self.player:getHandcardNum() >= 2 then	--第二次
			local discards = dimeng_discard(self, 2, cards)
			if #discards > 0 then
				local total_use_value = 0
				for _, id in ipairs(discards) do
					total_use_value = total_use_value + self:getUseValue(sgs.Sanguosha:getCard(id))
				end
				
				if total_use_value <= 10 then
					local card_str = ("ex_nihilo:xianwei[%s:%s]=%d+%d"):format("to_be_decided", 0, discards[1], discards[2])
					local skillcard = sgs.Card_Parse(card_str)
				
					assert(skillcard)
				
					return skillcard
				end
			end
		elseif self.player:getMark("&xianwei_count") == 2 and self.player:getHandcardNum() >= 3 then	--第三次
			local discards = dimeng_discard(self, 3, cards)
			if #discards > 0 then
				local total_use_value = 0
				for _, id in ipairs(discards) do
					total_use_value = total_use_value + self:getUseValue(sgs.Sanguosha:getCard(id))
				end
				
				if total_use_value <= 10 then
					local card_str = ("ex_nihilo:xianwei[%s:%s]=%d+%d+%d"):format("to_be_decided", 0, discards[1], discards[2], discards[3])
					local skillcard = sgs.Card_Parse(card_str)
				
					assert(skillcard)
				
					return skillcard
				end
			end
		end
	end
end




--嘲讽
sgs.ai_chaofeng.dongaili_xingtu = 0

--------------------------------------------------
--咏星
--------------------------------------------------

sgs.ai_cardneed.hunyin = function(to, card, self)
	return card:isKindOf("AOE") or card:isKindOf("Duel")
end

--------------------------------------------------
--扬歌
--------------------------------------------------

sgs.ai_skill_invoke.yangge = function(self, data)
	self.yangge_type = false
	if not self.player:isNude() then
		local god_salvation = sgs.Sanguosha:cloneCard("god_salvation", sgs.Card_NoSuit, 0)
		god_salvation:setSkillName("yangge")
		if self:willUse(self.player, god_salvation) then
			self.yangge_type = true
			return true
		end
	end
	
	local has_weak_enemy = false
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self:isWeak(enemy) and enemy:getHp() == 1 then
			has_weak_enemy = true
		end
	end
	
	local savage_assault = sgs.Sanguosha:cloneCard("savage_assault", sgs.Card_NoSuit, 0)
	savage_assault:setSkillName("yangge")
	if has_weak_enemy and self:willUse(self.player, savage_assault) then
		return true
	end
	
	return false
end

sgs.ai_skill_discard["yangge"] = function(self, discard_num, min_num, optional, include_equip)	--yun
	local cards = sgs.QList2Table(self.player:getCards("he"))
	local to_discard = {}
	if self.yangge_type == true then
		to_discard = dimeng_discard(self, 1, cards)
	end
	return to_discard
end




--嘲讽
sgs.ai_chaofeng.nanyinnai_maomaotou = 0

--------------------------------------------------
--隐游
--------------------------------------------------

sgs.ai_skill_use["@@yinyou"] = function(self, prompt, method)
	if self.player:getHandcardNum() < 2 then return "" end
    local card_ids = {}
	local cards = sgs.QList2Table(self.player:getCards("h"))
	local card, target = self:getCardNeedPlayer(cards, false)
	if target and target:isAlive() and card then	--有需要某牌的队友
		table.insert(card_ids, card:getEffectiveId())
		table.removeOne(cards, card)
		for _, hcard in ipairs(cards) do	--再给其所需的牌
			for _, askill in sgs.qlist(target:getVisibleSkillList(true)) do
				local callback = sgs.ai_cardneed[askill:objectName()]
				if type(callback)=="function" and callback(target, hcard, self) then
					table.insert(card_ids, hcard:getEffectiveId())
					table.removeOne(cards, hcard)
					break
				end
			end
			if #card_ids >= 2 and (self:getOverflow() - #card_ids) <= 0 then
				break
			end
		end
		self:sortByKeepValue(cards)
		cards = sgs.reverse(cards)
		for _,acard in ipairs(cards) do
			if (#cards > 2 or self:isWeak()) and (acard:isKindOf("Peach") or acard:isKindOf("Jink")) then	--从可给牌中剔除闪桃
				table.removeOne(cards, acard)
			end
		end
		cards = sgs.reverse(cards)
		for _,acard in ipairs(cards) do
			if #cards > 2 and self.player:hasSkill("xiange") and acard:isKindOf("TrickCard") then	--可以的话剔除一张最没用的锦囊
				table.removeOne(cards, acard)
				break
			end
		end
		for _,acard in ipairs(cards) do
			if not table.contains(card_ids, acard:getEffectiveId()) and (#card_ids < 2 or (self:getOverflow() - #card_ids) > 0) then
				table.insert(card_ids, acard:getEffectiveId())
			end
		end
		if #card_ids >= 2 then
			return "#yinyou:"..table.concat(card_ids, "+")..":->"..target:objectName()
		end
	elseif #self.friends_noself > 0 then	--没有需要某牌的队友
		self:sortByKeepValue(cards)
		cards = sgs.reverse(cards)
		for _,acard in ipairs(cards) do
			if (#cards > 2 or self:isWeak()) and (acard:isKindOf("Peach") or acard:isKindOf("Jink")) then	--从可给牌中剔除闪桃
				table.removeOne(cards, acard)
			end
		end
		cards = sgs.reverse(cards)
		for _,acard in ipairs(cards) do
			if #cards > 2 and self.player:hasSkill("xiange") and acard:isKindOf("TrickCard") then	--可以的话剔除一张最没用的锦囊
				table.removeOne(cards, acard)
				break
			end
		end
		if #cards >= 2 then
			local give_ids = dimeng_discard(self, math.max(2, self:getOverflow()), cards)
			if #give_ids >= 2 then
				self:sort(self.friends_noself, "defense")
				for _, friend in ipairs(self.friends_noself) do
					if self:canDraw(friend) then
						return "#yinyou:"..table.concat(give_ids, "+")..":->"..friend:objectName()
					end
				end
			end
		end
	end
end

sgs.ai_playerchosen_intention.yinyou = function(self, from, to)
	local intention = -10
	if (to:hasSkill("kongcheng") and to:isKongcheng()) or hasManjuanEffect(to) then
		intention = 0
	end
	sgs.updateIntention(from, to, intention)
end

--------------------------------------------------
--闲歌
--------------------------------------------------

local xiange_skill = {}
xiange_skill.name = "xiange"
table.insert(sgs.ai_skills, xiange_skill)
xiange_skill.getTurnUseCard = function(self, inclusive)
	if self.player:usedTimes("#xiange") < 1 then
		return sgs.Card_Parse("#xiange:.:")
	end
end
sgs.ai_skill_use_func["#xiange"] = function(card, use, self)
	local target, card_str
	local targets, friends, enemies = {}, {}, {}

	local hcards = self.player:getHandcards()
	local hand_trick
	local just_use = false
	for _, hcard in sgs.qlist(hcards) do
		if hcard:isKindOf("TrickCard") then
			hand_trick = true
			card_str = "#xiange:" .. hcard:getId() .. ":"
		end
	end
	if (hand_trick and self:getOverflow() > 0) or (self.player:getHp() > 3 and self:getOverflow() < 0) then
		just_use = true
	end
	if hand_trick or self.player:getHp() > 3 then
		if not card_str then card_str = "#xiange:.:" end
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self.player:canSlash(player) then
				table.insert(targets, player)

				if self:isFriend(player) then
					table.insert(friends, player)
				elseif self:isEnemy(player) and not self:doNotDiscard(player, "he", nil, 2) then
					table.insert(enemies, player)
				end
			end
		end
	else
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self.player:distanceTo(player) <= 1 then
				table.insert(targets, player)

				if self:isFriend(player) then
					table.insert(friends, player)
				elseif self:isEnemy(player) and not self:doNotDiscard(player, "he", nil, 2) then
					table.insert(enemies, player)
				end
			end
		end
	end

	if #targets == 0 then return end
	for _, player in ipairs(targets) do
		if not player:containsTrick("YanxiaoCard") and player:containsTrick("lightning") and self:getFinalRetrial(player) == 2 then
			target = player
			break
		end
	end
	if not target and #friends ~= 0 then
		for _, friend in ipairs(friends) do
			if not friend:containsTrick("YanxiaoCard") and not (friend:hasSkill("qiaobian") and not friend:isKongcheng())
			  and (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				target = friend
				break
			end
			if friend:getCards("e"):length() > 1 and self:hasSkills(sgs.lose_equip_skill, friend) then
				target = friend
				break
			end
		end
	end
	if not target and #enemies > 0 then
		self:sort(enemies, "defense")
		for _, enemy in ipairs(enemies) do
			if enemy:containsTrick("YanxiaoCard") and (enemy:containsTrick("indulgence") or enemy:containsTrick("supply_shortage")) then
				target = enemy
				break
			end
			if self:getDangerousCard(enemy) then
				target = enemy
				break
			end
			if not enemy:hasSkill("tuntian+zaoxian") then
				target = enemy
				break
			end
		end
	end
	if not target and #enemies > 0 and just_use then
		self:sort(enemies, "defense")
		for _, enemy in ipairs(enemies) do
			if enemy:getCardCount(true) >= 2 then
				target = enemy
				break
			end
		end
	end

	if not target or not SkillCanTarget(target, self.player, "xiange") then return end
	if not card_str then
		if self:isFriend(target) and self.player:getHp() > 2 then card_str = "#xiange:.:" end
	end

	if card_str then
		if use.to then
			if self:isFriend(target) then
				if not use.isDummy then target:setFlags("xiangeOK") end
			end
			use.to:append(target)
		end
		card_str = card_str.."->"..target:objectName()
		--self.player:speak(card_str)
		use.card = sgs.Card_Parse(card_str)
	end
end

sgs.ai_cardneed.xiange = function(to, card, self)
	return card:isKindOf("TrickCard")
end

sgs.ai_use_priority["xiange"] = sgs.ai_use_priority.Dismantlement + 0.1

sgs.ai_card_intention.xiange = function(self, card, from, tos)
	if #tos > 0 then
		for _,to in ipairs(tos) do
			if to:hasFlag("xiangeOK") then
				to:setFlags("-xiangeOK")
				sgs.updateIntention(from, to, -10)
			elseif not self:isFriend(from, to) then
				sgs.updateIntention(from, to, 10)
			end
		end
	end
	return 0
end




--嘲讽
sgs.ai_chaofeng.beiyouxiang_motaishouke = 0

--------------------------------------------------
--作业
--------------------------------------------------

sgs.ai_skill_playerchosen.zuoye = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	local to = findPlayerByFlag(self.room, "zuoye_target_AI")
	if to then		--选择第二步
		local targets = sgs.SPlayerList()
		for _, vic in sgs.qlist(self.room:getOtherPlayers(to)) do
			if to:canSlash(vic) then
				targets:append(vic)
			end
		end
		if not targets:isEmpty() then
			targets = sgs.QList2Table(targets)
			self:sort(targets, "defense")
			for _, target in ipairs(targets) do
				if self:isEnemy(target) then
					return target
				end
			end
		end
		self.player:speak("not_found")
	else			--选择第一步
		self:sort(targetlist, "defense")
		targetlist = sgs.reverse(targetlist)
		
		for _, p in ipairs(targetlist) do					--被翻面的队友
			if self:isFriend(p) and not p:faceUp() then
				return p
			end
		end
		for _, p in ipairs(targetlist) do					--不能出杀的敌人
			if self:isEnemy(p) and p:faceUp() then
				local targets = sgs.SPlayerList()
				for _, vic in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p:canSlash(vic) then
						targets:append(vic)
					end
				end
				if targets:isEmpty() then
					return p
				end
			end
		end
		for _, p in ipairs(targetlist) do					--有明杀的队友
			if self:isFriend(p) and getCardsNum("Slash", p, self.player) > 0 then
				for _, vic in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p:canSlash(vic) then
						return p
					end
				end
			end
		end
		for _, p in ipairs(targetlist) do					--未知牌多的队友
			local unknown_card_count = 0
			for _, cd in sgs.qlist(p:getHandcards()) do
				local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), p:objectName())
				if not cd:hasFlag("visible") then
					unknown_card_count = unknown_card_count + 1
				end
			end
			if self:isFriend(p) and unknown_card_count >= 3 then
				for _, vic in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p:canSlash(vic) then
						return p
					end
				end
			end
		end
		for _, p in ipairs(targetlist) do					--能杀虚弱队友的敌人
			if self:isEnemy(p) and p:faceUp() then
				local targets = sgs.SPlayerList()
				for _, vic in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p:canSlash(vic) and self:isFriend(p, vic) and self:isWeak(vic) then
						return p
					end
				end
			end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.zuoye = function(self, from, to)
	local target = findPlayerByFlag(self.room, "zuoye_target_AI")
	if target then		--第二步
		if not target:faceUp() then		--若第一步的玩家为背面则选谁都无所谓
			sgs.updateIntention(from, to, 10)
		end
	else				--第一步
		if not to:faceUp() then			--这里只计选背面角色的情况为友好（为了给人类玩家容错）
			sgs.updateIntention(from, to, -10)
		end
	end
end




--嘲讽
sgs.ai_chaofeng.ximoyou_jiweimowang = -2

--------------------------------------------------
--阴谋
--------------------------------------------------

local yinmou_skill={}
yinmou_skill.name="yinmou"
table.insert(sgs.ai_skills,yinmou_skill)
yinmou_skill.getTurnUseCard=function(self)
	if self.player:getMark("yinmou") == 1 then return end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards) do
		if acard:isBlack() and acard:isKindOf("TrickCard") and (self:getDynamicUsePriority(acard) <= 9 or self:getOverflow() > 0) then
			card = acard
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("duel:yinmou[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)

	return skillcard
end

sgs.ai_cardneed.yinmou = function(to, card, self)
	return card:isBlack() and card:isKindOf("TrickCard")
end

--------------------------------------------------
--幽炼
--------------------------------------------------

sgs.ai_skill_invoke.youlian = function(self, data)
	return true		--就烧，魂就完事了
end

sgs.ai_skill_cardask["@youlian_give"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards, self:isFriend(self.room:getCurrent()))
	for _, card in ipairs(cards) do
		if not card:isKindOf("BasicCard") then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end




--嘲讽
sgs.ai_chaofeng.shuangyue_yuezhishuangzi = 0

--------------------------------------------------
--月盈
--------------------------------------------------

sgs.ai_skill_use["@@yueying"] = function(self, prompt, method)
	self:updatePlayers()
	if self.player:getChangeSkillState("yueying") <= 1 then		--月盈杀
		local data = self.player:getTag("yueying_data")
		local move = data:toMoveOneTime()
		if move.card_ids:length() > 2 and not (self:willSkipPlayPhase() and self:getOverflow() > 0) then
			return
		elseif move.card_ids:length() == 2 and self.player:getPhase() ~= sgs.Player_NotActive then
			if (self.player:getPhase() < sgs.Player_Play and not self:willSkipPlayPhase()) or self.player:getPhase() == sgs.Player_Play then
				for _,card in sgs.qlist(self.player:getCards("h")) do
					if card:isKindOf("ExNihilo") or card:isKindOf("IronChain") then
						return
					end
				end
			end
			
			local average_use_value = 0
			for _,id in sgs.qlist(move.card_ids) do
				if sgs.Sanguosha:getCard(id):isKindOf("ExNihilo") and not self:willSkipPlayPhase() then
					return
				end
				average_use_value = average_use_value + self:getUseValue(sgs.Sanguosha:getCard(id))
			end
			average_use_value = average_use_value / move.card_ids:length()
			
			if average_use_value > 8 then
				return
			end
		elseif move.card_ids:length() == 1 then
			local card = sgs.Sanguosha:getCard(move.card_ids:first())
			if (card:isKindOf("Peach") or card:isKindOf("Analeptic") or card:isKindOf("Jink")) and self:isWeak() then
				return
			end
		end
		
		local target_card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:hasFlag("yueyingcard") then
				target_card:addSubcard(c:getId())
			end
		end
		target_card:setSkillName("yueying")
		if not target_card or target_card:subcardsLength() == 0 then return end
		
		local to = self:findPlayerToSlash(true, target_card, nil, false)		--距离限制、卡牌、角色限制、必须选择
		if to then
			return target_card:toString() .. "->" .. to:objectName()
		end
	else											--月盈无中生有
		local data = self.player:getTag("yueying_data")
		local move = data:toMoveOneTime()
		if move.card_ids:length() < 2 then
			return true
		elseif move.card_ids:length() == 2 then
			if (self.player:getPhase() < sgs.Player_Play and not self:willSkipPlayPhase() and self.player:getPhase() ~= sgs.Player_NotActive) or self.player:getPhase() == sgs.Player_Play then
				for _,card in sgs.qlist(self.player:getCards("h")) do
					if card:isKindOf("ExNihilo") or card:isKindOf("IronChain") then
						return
					end
				end
			end
			
			local average_use_value = 0
			for _,id in sgs.qlist(move.card_ids) do
				if sgs.Sanguosha:getCard(id):isKindOf("ExNihilo") and not self:willSkipPlayPhase() then
					return
				end
				average_use_value = average_use_value + self:getUseValue(sgs.Sanguosha:getCard(id))
			end
			average_use_value = average_use_value / move.card_ids:length()
			if average_use_value < 6 then
				return "#yueying:.:->"..self.player:objectName()
			end
		end
		return
	end
end




--嘲讽
sgs.ai_chaofeng.youte_lianxinmonv = -1

--------------------------------------------------
--链心
--------------------------------------------------

sgs.ai_skill_playerchosen.lianxin = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	for _, p in ipairs(targetlist) do
		if p:objectName() == self.player:objectName() and self.player:isWounded() then
			return p
		end
	end
	self:sort(targetlist, "defense")
	for _, p in ipairs(targetlist) do
		if self:isEnemy(p) then
			return p
		end
	end
end

sgs.ai_skill_invoke.lianxin = function(self, data)
	local objname = string.sub(data:toString(), 8, -1)	--截取choice:后面的部分(即player:objectName())
	local to = findPlayerByObjName(self.room, objname)
	if to and self:isFriend(to) then
		return true
	end
	return false
end

--------------------------------------------------
--惑炎
--------------------------------------------------
--这个怎么都写不好，算了



--嘲讽
sgs.ai_chaofeng.mien_duobiannvpu = 1

--------------------------------------------------
--全能？
--------------------------------------------------

local quanneng_skill = {}
quanneng_skill.name = "quanneng"
table.insert(sgs.ai_skills, quanneng_skill)
quanneng_skill.getTurnUseCard = function(self, inclusive)
	self:updatePlayers()
	if self.player:getMark("quanneng_used") > 0 then return end
	local handcards = sgs.QList2Table(self.player:getCards("h"))
	if self.player:getPile("wooden_ox"):length() > 0 then
		for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
			table.insert(handcards ,sgs.Sanguosha:getCard(id))
		end
	end
	self:sortByUseValue(handcards, true)
	local equipments = sgs.QList2Table(self.player:getCards("e"))
	self:sortByUseValue(equipments, true)
	local basic_cards = {}
	local basic_cards_count = 0
	local non_basic_cards = {}
	local use_cards = {}
	
	for _,c in ipairs(handcards) do
		if c:isKindOf("BasicCard") and not c:isKindOf("Peach") then
			basic_cards_count = basic_cards_count + 1
			table.insert(basic_cards, c:getEffectiveId())
		else
			table.insert(non_basic_cards, c:getEffectiveId())
		end
	end
	for _,e in ipairs(equipments) do
		if e:isKindOf("OffensiveHorse") then
			table.insert(non_basic_cards, e:getEffectiveId())
		end
		if self.player:hasArmorEffect("silver_lion") and self.player:isWounded() and self.player:getLostHp() >= 2 and e:isKindOf("SilverLion") then
			table.insert(non_basic_cards, e:getEffectiveId())
		end
	end
	if basic_cards_count < 3 then return end
	
	--if self.player:getMark("@quannengUsed") >= 3 then
		if #basic_cards > 0 then
			table.insert(use_cards, basic_cards[1])
		end
		if #use_cards == 0 then return end
	--[[else
		if #basic_cards > 0 and #non_basic_cards > 0 then
			table.insert(use_cards, basic_cards[1])
			table.insert(use_cards, non_basic_cards[1])
		elseif #basic_cards > 1 and #non_basic_cards == 0 then
			table.insert(use_cards, basic_cards[1])
			table.insert(use_cards, basic_cards[2])
		end
		if #use_cards ~= 2 then return end
	end]]
	
	if (self:isWeak() or self.player:getHp() <= 1) and self.player:isWounded() then
		return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "peach")
	end
	local slash = sgs.Sanguosha:cloneCard("slash")
	if self:getCardsNum("Slash") > 1 and not slash:isAvailable(self.player) then
		for _, enemy in ipairs(self.enemies) do
			if ((enemy:getHp() < 3 and enemy:getHandcardNum() < 3) or (enemy:getHandcardNum() < 2)) and self.player:canSlash(enemy) and not self:slashProhibit(slash, enemy, self.player)
				and self:slashIsEffective(slash, enemy, self.player) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
				return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "analeptic")
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
			local thunder_slash = sgs.Sanguosha:cloneCard("thunder_slash")
			local fire_slash = sgs.Sanguosha:cloneCard("fire_slash")
			if  not self:slashProhibit(fire_slash, enemy, self.player)and self:slashIsEffective(fire_slash, enemy, self.player) then
				return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "fire_slash")
			end
			if not self:slashProhibit(thunder_slash, enemy, self.player)and self:slashIsEffective(thunder_slash, enemy, self.player) then
				return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "thunder_slash")
			end
			if not self:slashProhibit(slash, enemy, self.player)and self:slashIsEffective(slash, enemy, self.player) then
				return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "slash")
			end
		end
	end
	if self.player:isWounded() and (self:getOverflow() > 0 or self.player:getPhase() ~= sgs.Player_Play) then
		return sgs.Card_Parse("#quanneng:" .. table.concat(use_cards, "+") .. ":" .. "peach")
	end
end

sgs.ai_skill_use_func["#quanneng"] = function(card, use, self)
	if self.player:getMark("quanneng_used") > 0 then return end
	local userstring = card:toString()
	userstring = (userstring:split(":"))[4]
	local quannengcard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	quannengcard:setSkillName("quanneng")
	self:useBasicCard(quannengcard, use)
	if not use.card then return end
	use.card = card
end

sgs.ai_use_priority["quanneng"] = 3
sgs.ai_use_value["quanneng"] = 3

sgs.ai_view_as["quanneng"] = function(card, player, card_place, class_name)
	if player:getMark("quanneng_used") > 0 then return end
	local classname2objectname = {
		["Slash"] = "slash", ["Jink"] = "jink",
		["Peach"] = "peach", ["Analeptic"] = "analeptic",
		["FireSlash"] = "fire_slash", ["ThunderSlash"] = "thunder_slash"
	}
	local name = classname2objectname[class_name]
	if not name then return end
	local no_have = true
	local cards = player:getCards("he")
	if player:getPile("wooden_ox"):length() > 0 then
		for _, id in sgs.qlist(player:getPile("wooden_ox")) do
			cards:prepend(sgs.Sanguosha:getCard(id))
		end
	end
	for _,c in sgs.qlist(cards) do
		if c:isKindOf(class_name) then
			no_have = false
			break
		end
	end
	if not no_have then return end
	if class_name == "Peach" and player:getMark("Global_PreventPeach") > 0 then return end
	
	local handcards = sgs.QList2Table(player:getCards("h"))
	if player:getPile("wooden_ox"):length() > 0 then
		for _, id in sgs.qlist(player:getPile("wooden_ox")) do
			table.insert(handcards ,sgs.Sanguosha:getCard(id))
		end
	end
	local equipments = sgs.QList2Table(player:getCards("e"))
	local basic_cards = {}
	local non_basic_cards = {}
	local use_cards = {}
	
	for _,c in ipairs(handcards) do
		if c:isKindOf("BasicCard") and not c:isKindOf("Peach") then
			table.insert(basic_cards, c:getEffectiveId())
		else
			table.insert(non_basic_cards, c:getEffectiveId())
		end
	end
	for _,e in ipairs(equipments) do
		if not (e:isKindOf("Armor") or e:isKindOf("DefensiveHorse")) and not (e:isKindOf("WoodenOx") and player:getPile("wooden_ox"):length() > 0) then
			table.insert(non_basic_cards, e:getEffectiveId())
		end
		if player:hasArmorEffect("silver_lion") and player:isWounded() and player:getLostHp() >= 2 and e:isKindOf("SilverLion") then
			table.insert(non_basic_cards, e:getEffectiveId())
		end
	end
	
	--if player:getMark("@quannengUsed") >= 3 then
		if #basic_cards > 0 then
			table.insert(use_cards, basic_cards[1])
		end
		if #use_cards == 0 then return end
	--[[else
		if #basic_cards > 0 and #non_basic_cards > 0 then
			table.insert(use_cards, basic_cards[1])
			table.insert(use_cards, non_basic_cards[1])
		elseif #basic_cards > 1 and #non_basic_cards == 0 then
			table.insert(use_cards, basic_cards[1])
			table.insert(use_cards, basic_cards[2])
		end
		if #use_cards ~= 2 then return end
	end]]
	
	--if player:getMark("@quannengUsed") >= 3 then
		return (name..":quanneng[%s:%s]=%d"):format(sgs.Card_NoSuit, 0, use_cards[1])
	--[[else
		return (name..":quanneng[%s:%s]=%d+%d"):format(sgs.Card_NoSuit, 0, use_cards[1], use_cards[2])
	end]]
end

sgs.ai_cardneed.quanneng = function(to, card, self)
	return card:isKindOf("BasicCard")
end



--嘲讽
sgs.ai_chaofeng.huaman_wunianhuajiang = 1

--------------------------------------------------
--结硕
--------------------------------------------------

local jieshuo_skill={}
jieshuo_skill.name="jieshuo"
table.insert(sgs.ai_skills,jieshuo_skill)
jieshuo_skill.getTurnUseCard = function(self, inclusive)
	if self.player:usedTimes("#jieshuo") < 1 and not self.player:isKongcheng() then
		return sgs.Card_Parse("#jieshuo:.:")
	end
end
sgs.ai_skill_use_func["#jieshuo"] = function(card, use, self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and SkillCanTarget(friend, self.player, "jieshuo") then
			local cards = self.player:getCards("h")
			cards = sgs.QList2Table(cards)
			local ids = dimeng_discard(self, 1, cards)
			if use.to then
				use.to:append(friend)
			end
			card_str = "#jieshuo:"..ids[1]..":->"..friend:objectName()
			use.card = sgs.Card_Parse(card_str)
			break
		end 
	end
end

sgs.ai_use_priority["jieshuo"] = 8

sgs.ai_card_intention.jieshuo = -10

--------------------------------------------------
--春泥
--------------------------------------------------

sgs.ai_skill_use["@@chunni"] = function(self, prompt)
    local targets = {}
    for _, friend in ipairs(self.friends) do
        if friend:isWounded() and friend:objectName() ~= self.player:objectName() then
            table.insert(targets, friend:objectName())
        end 
    end
	if #targets > 0 then
		return "#chunni:.:->"..table.concat(targets, "+")
	end
end



--嘲讽
sgs.ai_chaofeng.xibeier_sanjueweibian = 0

--------------------------------------------------
--化武
--------------------------------------------------

sgs.ai_skill_invoke.huawu = function(self, data)
	if not self.player:getWeapon() and self.player:getEquips():length() <= 2 then
		return true
	end
end



--嘲讽
sgs.ai_chaofeng.leidi_cuilianzhiyuan = 0

--------------------------------------------------
--浴火
--------------------------------------------------

sgs.ai_skill_choice.yuhuo = function(self, choices)
	if self:isWeak() and self.player:isWounded() then
		return "yuhuo_recover"
	end
	return "yuhuo_draw"
end

--------------------------------------------------
--凭代
--------------------------------------------------

sgs.ai_skill_invoke.pingdai = function(self, data)
	local from = data:toPlayer()
	if self:isFriend(from) then
		if self:needToThrowArmor(from) or self:hasCrossbowEffect(from) or from:hasSkills(double_slash_skill) or self:hasTemporaryCard(from) then
			return true
		end
		if (from:getHandcardNum() >= 4 or self:getOverflow(from) > 0) and not self:isWeak(from) then
			return true
		end
	elseif self:isEnemy(from) and not self:needToThrowArmor(from) and not self:hasCrossbowEffect(from) and not from:hasSkills(double_slash_skill) and not self:hasTemporaryCard(from) then
		if from:getHandcardNum() <= 2 or (self:getOverflow(from) < 0 and self:isWeak(from)) then
			return true
		end
	end
	return false
end



--嘲讽
sgs.ai_chaofeng.lafa_duoluotianshi = 0

--------------------------------------------------
--废宅
--------------------------------------------------

sgs.ai_skill_invoke.feizhai = function(self, data)
	if self.shengguang_used then		--发动圣光的回合不用废宅
		self.shengguang_used = false
		return false
	end
	local has_equip_area = 0
	for i = 0, 4 do
		if self.player:hasEquipArea(i) then
			has_equip_area = has_equip_area + 1
		end
	end
	if has_equip_area == 5 and not self:willSkipPlayPhase() then
		return true
	elseif has_equip_area >= 3 and self.player:getHandcardNum() <= 2 and not self:willSkipPlayPhase() then
		return true
	elseif has_equip_area > 0 and self:isWeak() and self:getHandcardNum() <= 3 and not self:willSkipPlayPhase() then
		return true
	end
	return false
end

sgs.ai_skill_choice.feizhai = function(self, choices)
	local items = choices:split("+")
    if #items == 1 then
        return items[1]
	else
		if table.contains(items, "jueyan4") then
			return "jueyan4"
		elseif table.contains(items, "jueyan3") then
			return "jueyan3"
		elseif table.contains(items, "jueyan2") then
			return "jueyan2"
		elseif table.contains(items, "jueyan1") then
			return "jueyan1"
		elseif table.contains(items, "jueyan0") then
			return "jueyan0"
		end
	end
end

--------------------------------------------------
--圣光
--------------------------------------------------

sgs.ai_skill_invoke.shengguang = function(self, data)
	if self:isWeak(self.player, true) then
		self.shengguang_used = true
		return true
	end
end




--嘲讽
sgs.ai_chaofeng.youlingzichen_dianziyouling = -2

--------------------------------------------------
--竭心
--------------------------------------------------

sgs.ai_skill_playerchosen.jiexin = function(self, targetlist)
	local target = self:findPlayerToDraw(true, 2)
	targetlist = sgs.QList2Table(targetlist)
	for _, p in ipairs(targetlist) do
		if p:objectName() == target:objectName() then
			return p
		end
	end
	return self.player
end

sgs.ai_playerchosen_intention.jiexin = function(self, from, to)
	local intention = -10
	sgs.updateIntention(from, to, intention)
end

sgs.ai_need_damaged.jiexin = function (self, attacker, player)
	if not self:isWeak() or self:getAllPeachNum() > 0 then
		return true
	end
	return false
end




--嘲讽
sgs.ai_chaofeng.xiaoxi_chixingai = 0

--------------------------------------------------
--精算（不会真有人凑32 64吧，不会吧不会吧）
--------------------------------------------------

local jingsuan_skill={}
jingsuan_skill.name="jingsuan"
table.insert(sgs.ai_skills,jingsuan_skill)
jingsuan_skill.getTurnUseCard = function(self, inclusive)
	if self.player:getMark("@byte") > 16 then
		return sgs.Card_Parse("#jingsuan:.:")
	end
end
sgs.ai_skill_use_func["#jingsuan"] = function(card, use, self)
	local target = self:findPlayerToDiscard("h", false, true, players, return_table)
	if target then
		if use.to then
			use.to:append(target)
		end
		card_str = "#jingsuan:.:->"..target:objectName()
		use.card = sgs.Card_Parse(card_str)
	end
end

sgs.ai_use_priority["jingsuan"] = 10.1

sgs.ai_card_intention.jingsuan = function(self, from, to)
	local intention = 10
	if not self:needKongcheng(to) and not to:hasSkills(sgs.lose_card_skills) then
		sgs.updateIntention(from, to, intention)
	end
end




--嘲讽
sgs.ai_chaofeng.xiaotao_tauxingai = 0

--------------------------------------------------
--散热
--------------------------------------------------

sgs.ai_skill_playerchosen.sanre = function(self, targetlist)
	local target = self:findPlayerToDiscard("hej", false, true, players, return_table)
	if target then
		targetlist = sgs.QList2Table(targetlist)
		for _, p in ipairs(targetlist) do
			if p:objectName() == target:objectName() then
				return p
			end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.sanre = function(self, from, to)
	local intention = 10
	if not self:needKongcheng(to) and not to:hasSkills(sgs.lose_card_skills) and not to:hasSkills(sgs.need_equip_skill) then
		sgs.updateIntention(from, to, intention)
	end
end

--------------------------------------------------
--变频
--------------------------------------------------

sgs.ai_skill_use["@@bianpin"] = function(self, prompt, method)
	self:updatePlayers()
	local trigger = false
	if self.player:getChangeSkillState("bianpin") <= 1 then		--变频①
		local use_other_count = 0
		local use_self_count = 0
		local slash_used = false
		local peach_used = 0
		for _, cd in sgs.qlist(self.player:getHandcards()) do
			if cd:isKindOf("TrickCard") and not cd:isKindOf("ExNihilo") then
				if self:willUse(self.player, cd) then
					use_other_count = use_other_count + 1
				end
			elseif cd:isKindOf("Slash") then
				if self:willUse(self.player, cd) and (not slash_used or self:hasCrossbowEffect()) then
					slash_used = true
					use_other_count = use_other_count + 1
				end
			elseif cd:isKindOf("EquipCard") or cd:isKindOf("ExNihilo") or cd:isKindOf("Analeptic") then
				use_self_count = use_self_count + 1
			elseif cd:isKindOf("Peach") and peach_used < self.player:getLostHp() then
				use_self_count = use_self_count + 1
				peach_used = peach_used + 1
			end
			if (self.player:getHandcardNum() - use_self_count <= self.player:getMaxCards() + 1) and use_other_count <= 3 then
				trigger = true
			end
		end
	else											--变频②
		local use_other_count = 0
		local use_self_count = 0
		local slash_used = false
		local peach_used = 0
		for _, cd in sgs.qlist(self.player:getHandcards()) do
			if cd:isKindOf("EquipCard") or cd:isKindOf("ExNihilo") or cd:isKindOf("Analeptic") then
				if self:willUse(self.player, cd) then
					use_self_count = use_self_count + 1
				end
			elseif cd:isKindOf("Peach") and peach_used < self.player:getLostHp() then
				use_self_count = use_self_count + 1
				peach_used = peach_used + 1
			end
			if use_self_count <= 3 then
				trigger = true
			end
		end
	end
	if trigger then
		return "#bianpin:.:->"..self.player:objectName()
	end
	return
end




--嘲讽
sgs.ai_chaofeng.xiaorou_rhoxingai = -1

--------------------------------------------------
--清冷
--------------------------------------------------

sgs.ai_skill_playerchosen.qingleng = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	self:sort(targetlist, "defense")
	for _, p in ipairs(targetlist) do
		if self:isFriend(p) then
			if p:hasSkills("sanre|yueying|xingyi|guobao") or self:needToThrowArmor() then
				return p
			end
		elseif self:isEnemy(p) and not p:hasSkills(sgs.lose_equip_skill.."|"..sgs.lose_card_skills) then
			return p
		end
	end
	for _, p in ipairs(targetlist) do
		if self:isFriend(p) then
			if not self:isWeak(p) and p:getHandcardNum() >= 3 then
				return p
			end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.qingleng = function(self, from, to)
	local intention = 0
	sgs.updateIntention(from, to, intention)
end

--[[sgs.ai_skill_cardchosen.qingleng = function(self, who, flags)	--先放弃思考
	if self:isFriend(who) then
		if #self.enemies > 0 then
			if self:needToThrowArmor(who) and who:getArmor() then
				return who:getArmor()
			end
			if who:hasEquip() then
				local equips = sgs.QList2Table(who:getCards("e"))
				self:sortByKeepValue(equips)
				for _, equip in ipairs(equips) do
					return equip
				end
			end
			if not who:isKongcheng() and not hasManjuanEffect(who) then
				return who:getRandomHandCard()
			end
		else
			if not who:isKongcheng() and not hasManjuanEffect(who) then
				return who:getRandomHandCard()
			end
		end
		local cards = who:getCards("he")
		return cards:at(math.random(0, cards:length() - 1))
	end
	
	if #self.friends_noself == 0 then
		if who:hasEquip() then
			local equips = sgs.QList2Table(who:getCards("e"))
			self:sortByKeepValue(equips)
			for _, equip in ipairs(equips) do
				if not (equip:isKindOf("Armor") and self:isEnemy(who) and self:needToThrowArmor(who)) then
					return equip
				end
			end
		end
	else
		if not who:isKongcheng() then
			return who:getRandomHandCard()
		end
	end
	
	local cards = who:getCards("he")
	return cards:at(math.random(0, cards:length() - 1))
end]]

sgs.ai_skill_choice["qingleng"] = function(self, choices, data)
	local items = choices:split("+")
    if #items == 1 then
        return items[1]
	else
		local from = findPlayerByFlag(self.room, "qingleng_from_AI")
		
		local target_card
		for _, cd in sgs.qlist(self.player:getHandcards()) do
			if cd:hasFlag("qingleng") then
				target_card = cd
				break
			end
		end
		
		if table.contains(items, "qingleng_destroy") then
			if from and self:isFriend(from) then
				if not (target_card and (target_card:isKindOf("ExNihilo") or (target_card:isKindOf("Peach") and self:isWeak()) or target_card:isKindOf("SilverLion"))) then
					return "qingleng_destroy"
				end
			elseif self.player:hasSkills("sanre") then
				return "qingleng_destroy"
			end
		end
		if table.contains(items, "qingleng_get") then
			return "qingleng_get"
		end
		if table.contains(items, "qingleng_use") then
			self:updatePlayers()
			self:sort(self.enemies, "defense")
			if target_card then
				if target_card:targetFixed() then
					if target_card:isKindOf("EquipCard") then
						local equip_index = target_card:getRealCard():toEquipCard():location()
						if self.player:getEquip(equip_index) == nil and self.player:hasEquipArea(equip_index) then
							return "qingleng_use"
						end
					end
					if target_card:isKindOf("Armor") then
						local equip_index = target_card:getRealCard():toEquipCard():location()
						if self.player:getEquip(equip_index) ~= nil and self.player:hasEquipArea(equip_index) and self:needToThrowArmor() then
							return "qingleng_use"
						end
					end
					if target_card:isKindOf("SavageAssault") then
						local savage_assault = sgs.Sanguosha:cloneCard("SavageAssault")
						if self:getAoeValue(savage_assault) > 0 then
							return "qingleng_use"
						end
					end
					if target_card:isKindOf("ArcheryAttack") then
						local archery_attack = sgs.Sanguosha:cloneCard("ArcheryAttack")
						if self:getAoeValue(archery_attack) > 0 then
							return "qingleng_use"
						end
					end
					if target_card:isKindOf("Peach") and self.player:isWounded() then
						return "qingleng_use"
					end
					if target_card:isKindOf("ExNihilo") then
						return "qingleng_use"
					end
				elseif target_card:isKindOf("TrickCard") then
					local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
					self:useTrickCard(target_card, dummyuse)
					if not dummyuse.to:isEmpty() then
						return "qingleng_use"
					end
				elseif target_card:isKindOf("Slash") then
					local slash = target_card
					for _,enemy in ipairs(self.enemies) do	--yun
						if not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) and self.player:canSlash(enemy, slash, true) then
							return "qingleng_use"
						end
					end
				end
				if (self:needKongcheng() and self.player:getHandcardNum() == 1) or hasManjuanEffect(self.player) then
					return "qingleng_use"
				end
			end
		end
	end
	return "qingleng_destroy"
end

sgs.ai_skill_use["@@qingleng!"] = function(self, prompt, method)
	self:updatePlayers()
	self:sort(self.enemies, "defense")
	
	local target_cards = {}
	for _, cd in sgs.qlist(self.player:getHandcards()) do
		if cd:hasFlag("qingleng") then
			table.insert(target_cards, cd)
		end
	end
	if #target_cards == 0 then return end
	
	self:sortByUseValue(target_cards, true)
	target_cards = sgs.reverse(target_cards)
	
	for _,target_card in ipairs(target_cards) do
		if target_card:targetFixed() then
			return target_card:toString()
		else
			if target_card:isKindOf("Slash") then	--yun
				local to = self:findPlayerToSlash(true, target_card, nil, true)		--距离限制、卡牌、角色限制、必须选择
				return target_card:toString() .. "->" .. to:objectName()
			end
			local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
			self:useTrickCard(target_card, dummyuse)
			local targets = {}
			if not dummyuse.to:isEmpty() then
				for _, p in sgs.qlist(dummyuse.to) do
					table.insert(targets, p:objectName())
				end
				return target_card:toString() .. "->" .. table.concat(targets, "+")
			else		--强制选择随机目标
				local targets_list = sgs.PlayerList()
				for _, target in sgs.qlist(self.room:getAlivePlayers()) do
					targets_list:append(target)
				end
				local targets = {}
				for _,p in sgs.qlist(self.room:getAlivePlayers()) do
					if target_card:targetFilter(targets_list, p, self.player) and not sgs.Self:isProhibited(p, target_card) then
						table.insert(targets, p:objectName())
					end
				end
				if #targets > 0 then
					return target_card:toString() .. "->" .. targets[math.random(1,#targets)]
				end
			end
		end
	end
	return "."
end

--------------------------------------------------
--情柔
--------------------------------------------------

sgs.ai_skill_invoke.qingrou = function(self, data)
	local target = data:toPlayer()
	if not target then return false end
	if self:isFriend(target) then
		return true
	end
	return false
end

sgs.ai_skill_cardask["@qingrou_give"] = function(self, data, pattern, target)
	if self:needToThrowArmor() and self.player:getArmor() then
		return "$" .. self.player:getArmor():getEffectiveId()
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards, self:isFriend(self.room:getCurrent()))
	for _, card in ipairs(cards) do
		return "$" .. card:getEffectiveId()
	end
	return "$" .. cards[1]:getEffectiveId()
end




--嘲讽
sgs.ai_chaofeng.lanyin_yuezhigongzhutu = 0

--------------------------------------------------
--云谣
--------------------------------------------------

sgs.ai_view_as.yunyao = function(card, player, card_place)
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand or card_place == sgs.Player_PlaceEquip or (card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "wooden_ox") then
		if player:getMark("@yunyueyao_using_black") + player:getMark("@yunyueyao_using_red") == 0 and not card:isKindOf("BasicCard") then
			return ("nullification:yunyao[%s:%s]=%d"):format("to_be_decided", 0, card_id)
		end
	end
end

sgs.ai_cardneed.yunyao = function(to, card, self)
	return not card:isKindOf("BasicCard")
end

--------------------------------------------------
--国宝
--------------------------------------------------

sgs.ai_skill_playerchosen.guobao = function(self, targetlist)
	self:sort(self.friends, "defense")
	local target = self.friends[1]
	if target then
		targetlist = sgs.QList2Table(targetlist)
		for _, p in ipairs(targetlist) do
			if p:objectName() == target:objectName() then
				return p
			end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.guobao = function(self, from, to)
	local intention = -10
	sgs.updateIntention(from, to, intention)
end




--嘲讽
sgs.ai_chaofeng.toufa_shiyixian = 0

--------------------------------------------------
--流易
--------------------------------------------------

sgs.ai_skill_invoke.liuyi = function(self, data)
	local data = self.player:getTag("liuyi")
	local judge = data:toJudge()
	if self:needRetrial(judge) then
		return true
	elseif self:isFriend(judge.who) then
		if not judge:isGood() then
			return true
		else
			return false
		end
	elseif self:isEnemy(judge.who) then
		if judge:isGood() then
			return true
		else
			return false
		end
	end
	return false
end

--------------------------------------------------
--诲谕
--------------------------------------------------

sgs.ai_skill_invoke.huiyu = function(self, data)
	local target = data:toPlayer()
	if not target then return false end
	if target:objectName() == self.player:objectName() then
		return true
	elseif self:isFriend(target) then
		return true
	end
	return false
end

sgs.ai_skill_askforag.huiyu = function(self, card_ids)
	local cards = {}
	for _, id in ipairs(card_ids) do
		if sgs.Sanguosha:getCard(id):isBlack() then
			return id
		end
	end
	for _, id in ipairs(card_ids) do
		return id
	end
	return -1
end




--嘲讽
sgs.ai_chaofeng.bijujieyi_senluozhilantu = 1

--------------------------------------------------
--病娇
--------------------------------------------------

sgs.ai_skill_cardask["@bingjiao_give"] = function(self, data, pattern, target)
	local current = self.room:getCurrent()
	if self:isEnemy(current) and (self:isWeak(current) or self:getOverflow(current) > 0) then
		local cards = sgs.QList2Table(self.player:getCards("h"))
		self:sortByKeepValue(cards, self:isFriend(current))
		for _, card in ipairs(cards) do
			if card:getSuit() == sgs.Card_Heart then
				return "$" .. card:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_skill_choice.bingjiao = function(self, choices)
	if self.player:getHp() == 1 then
		return "bingjiao_skip"
	elseif self:getOverflow() >= 2 then
		return "bingjiao_damage"
	end
	return "bingjiao_skip"
end

sgs.ai_cardneed.bingjiao = function(to, card, self)
	return card:getSuit() == sgs.Card_Heart
end

sgs.bingjiao_suit_value = {
	heart = 6,
}




--嘲讽
sgs.ai_chaofeng.chushuang_jinglingxuediao = 2

--嘲讽
sgs.ai_chaofeng.xiyue_shenshengxuantu = 0

--------------------------------------------------
--月引
--------------------------------------------------

local yueyinvs_skill = {}
yueyinvs_skill.name = "yueyinvs"
table.insert(sgs.ai_skills,yueyinvs_skill)
yueyinvs_skill.getTurnUseCard=function(self)
	if self:needBear() then return end
	if not self.player:isKongcheng() and self.player:getHandcardNum() > self.player:getHp() and not self:needBear() then
		return sgs.Card_Parse("#yueyinvs:.:")
	end
end

sgs.ai_skill_use_func["#yueyinvs"] = function(card, use, self)
	local handcards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(handcards, true)
	local give_card = handcards[1]
	if give_card then
		for _, p in sgs.qlist(self.room:findPlayersBySkillName("yueyin")) do
			if self:isFriend(p) and p:objectName() ~= self.player:objectName() and self:getOverflow() > 0 and p:getMark("yueyin_used") == 0 then
				use.card = sgs.Card_Parse("#yueyinvs:" .. give_card:getId() .. ":->" .. p:objectName())
				if use.to then
					use.to:append(p)
				end
				return
			end
		end
	end
end

sgs.ai_use_value.yueyinvs = 2.5
sgs.ai_use_priority.yueyinvs = 9
sgs.ai_card_intention.yueyinvs = -10

--------------------------------------------------
--星移
--------------------------------------------------

sgs.ai_skill_choice["xingyi"] = function(self, choices, data)
	self:updatePlayers()
	self:sort(self.enemies, "defense")
	
	local target_cards = {}
	for _, cd in sgs.qlist(self.player:getHandcards()) do
		if cd:hasFlag("xingyi") then
			table.insert(target_cards, cd)
		end
	end
	if #target_cards == 0 then return end
	
	self:sortByUseValue(target_cards, true)
	target_cards = sgs.reverse(target_cards)
	
	for _,target_card in ipairs(target_cards) do
		if target_card:targetFixed() then
			if target_card:isKindOf("EquipCard") then
				local equip_index = target_card:getRealCard():toEquipCard():location()
				if self.player:getEquip(equip_index) == nil and self.player:hasEquipArea(equip_index) then
					return "xingyi_use"
				end
			end
			if target_card:isKindOf("Armor") then
				local equip_index = target_card:getRealCard():toEquipCard():location()
				if self.player:getEquip(equip_index) ~= nil and self.player:hasEquipArea(equip_index) and self:needToThrowArmor() then
					return "xingyi_use"
				end
			end
			if target_card:isKindOf("SavageAssault") then
				local savage_assault = sgs.Sanguosha:cloneCard("SavageAssault")
				if self:getAoeValue(savage_assault) > 0 then
					return "xingyi_use"
				end
			end
			if target_card:isKindOf("ArcheryAttack") then
				local archery_attack = sgs.Sanguosha:cloneCard("ArcheryAttack")
				if self:getAoeValue(archery_attack) > 0 then
					return "xingyi_use"
				end
			end
			if target_card:isKindOf("Peach") and self.player:isWounded() then
				return "xingyi_use"
			end
			if target_card:isKindOf("ExNihilo") then
				return "xingyi_use"
			end
		elseif target_card:isKindOf("TrickCard") then
			local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
			self:useTrickCard(target_card, dummyuse)
			if not dummyuse.to:isEmpty() then
				return "xingyi_use"
			end
		elseif target_card:isKindOf("Slash") then
			local slash = target_card
			for _,enemy in ipairs(self.enemies) do	--yun
				if not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) and self.player:canSlash(enemy, slash, true) then
					return "xingyi_use"
				end
			end
		end
		if (self:needKongcheng() and self.player:getHandcardNum() == 1) or hasManjuanEffect(self.player) then
			return "xingyi_use"
		end
	end
	return "cancel"
end

sgs.ai_skill_use["@@xingyi!"] = function(self, prompt, method)
	self:updatePlayers()
	self:sort(self.enemies, "defense")
	
	local target_cards = {}
	for _, cd in sgs.qlist(self.player:getHandcards()) do
		if cd:hasFlag("xingyi") then
			table.insert(target_cards, cd)
		end
	end
	if #target_cards == 0 then return end
	
	self:sortByUseValue(target_cards, true)
	target_cards = sgs.reverse(target_cards)
	
	for _,target_card in ipairs(target_cards) do
		if target_card:targetFixed() then
			return target_card:toString()
		else
			if target_card:isKindOf("Slash") then	--yun
				local to = self:findPlayerToSlash(true, target_card, nil, true)		--距离限制、卡牌、角色限制、必须选择
				return target_card:toString() .. "->" .. to:objectName()
			end
			local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
			self:useTrickCard(target_card, dummyuse)
			local targets = {}
			if not dummyuse.to:isEmpty() then
				for _, p in sgs.qlist(dummyuse.to) do
					table.insert(targets, p:objectName())
				end
				return target_card:toString() .. "->" .. table.concat(targets, "+")
			else		--强制选择随机目标
				local targets_list = sgs.PlayerList()
				for _, target in sgs.qlist(self.room:getAlivePlayers()) do
					targets_list:append(target)
				end
				local targets = {}
				for _,p in sgs.qlist(self.room:getAlivePlayers()) do
					if target_card:targetFilter(targets_list, p, self.player) and not sgs.Self:isProhibited(p, target_card) then
						table.insert(targets, p:objectName())
					end
				end
				if #targets > 0 then
					return target_card:toString() .. "->" .. targets[math.random(1,#targets)]
				end
			end
		end
	end
	return "."
end




--嘲讽
sgs.ai_chaofeng.pengshanmu_zhuoluofengchan = 0

--------------------------------------------------
--郁生
--------------------------------------------------

sgs.ai_skill_invoke.yusheng = function(self, data)
	return true
end




--嘲讽
sgs.ai_chaofeng.xiaheyi_yinyangshi = 1

--------------------------------------------------
--结印
--------------------------------------------------

local jieyin_v_skill={}
jieyin_v_skill.name="jieyin_v"
table.insert(sgs.ai_skills,jieyin_v_skill)
jieyin_v_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local card
	self:sortByUseValue(cards, true)

	local slash = self:getCard("FireSlash") or self:getCard("ThunderSlash") or self:getCard("Slash")
	if slash then
		local dummy_use = { isDummy = true }
		self:useBasicCard(slash, dummy_use)
		if not dummy_use.card then slash = nil end
	end

	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Spade then
			local shouldUse = true
			if self:getUseValue(acard) > sgs.ai_use_value.IronChain and acard:getTypeId() == sgs.Card_TypeTrick then
				local dummy_use = { isDummy = true }
				self:useTrickCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if acard:getTypeId() == sgs.Card_TypeEquip then
				local dummy_use = { isDummy = true }
				self:useEquipCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if shouldUse and (not slash or slash:getEffectiveId() ~= acard:getEffectiveId()) then
				card = acard
				break
			end
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:jieyin_v[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_cardneed.jieyin_v = function(to, card)
	return card:getSuit() == sgs.Card_Spade and to:getHandcardNum() <= 2
end

--------------------------------------------------
--鹤唳
--------------------------------------------------

sgs.ai_skill_invoke.heli = function(self, data)
	return true		--就烧，魂就完事了
end

sgs.ai_skill_cardask["@heli_give"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards, self:isFriend(self.room:getCurrent()))
	for _, card in ipairs(cards) do
		if card:isBlack() then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end




--嘲讽
sgs.ai_chaofeng.ailulu_hunaoxiaoxiongmao = 2

--------------------------------------------------
--藏宝
--------------------------------------------------

local cangbao_skill = {}
cangbao_skill.name = "cangbao"
table.insert(sgs.ai_skills, cangbao_skill)
cangbao_skill.getTurnUseCard = function(self)
	if self.player:usedTimes("#cangbao") >= 1 then return end
	return sgs.Card_Parse("#cangbao:.:")
end
sgs.ai_skill_use_func["#cangbao"] = function(card, use, self)
	local targets = sgs.SPlayerList()
	for _, vic in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not vic:isNude() and vic:getHandcardNum() > self.player:getHandcardNum() and SkillCanTarget(vic, self.player, "cangbao") then
			targets:append(vic)
		end
	end
	if not targets:isEmpty() then
		--self:sort(targets, "defense")
		local target = self:findPlayerToDiscard("he", false, false, targets, false)
		if target then
			if use.to then
				use.to:append(target)
			end
			card_str = "#cangbao:.:->"..target:objectName()
			use.card = sgs.Card_Parse(card_str)
		end
	end
end

sgs.ai_use_value["cangbao"] = 1 --卡牌使用价值
sgs.ai_use_priority["cangbao"] = sgs.ai_use_priority.Slash + 0.2 --卡牌使用优先级

sgs.ai_choicemade_filter.cardChosen["cangbao"] = sgs.ai_choicemade_filter.cardChosen.snatch

sgs.ai_card_intention.cangbao = function(self, card, from, tos)
    local to = tos[1]
    local intention = 10
    if self:needKongcheng(to) then
        intention = 0
    end
    sgs.updateIntention(from, to, intention)
end




--嘲讽
sgs.ai_chaofeng.youyueying_fuyunzuofutong = 1

--------------------------------------------------
--福佑
--------------------------------------------------

sgs.ai_skill_invoke["fuyou"] = function(self, data)	--yun
	local current_dying_player = data:toPlayer()
	if current_dying_player and self:canDraw(current_dying_player) then
		if self:isFriend(current_dying_player) then
			sgs.updateIntention(self.player, current_dying_player, -10)
			return true
		else
			sgs.updateIntention(self.player, current_dying_player, 10)
		end
	end
	return false
end

sgs.ai_choicemade_filter.skillInvoke.fuyou = function(self, player, promptlist)	--yun
	local dying = findPlayerByFlag(self.room, "fuyou_dying_AI")
	if dying and self:canDraw(dying, player) then
		if promptlist[#promptlist] == "yes" then
			sgs.updateIntention(player, dying, -10) 
		elseif promptlist[#promptlist] == "no" then
			sgs.updateIntention(player, dying, 10)
		end
	end
end

--------------------------------------------------
--满愿
--------------------------------------------------

local manyuan_skill = {}
manyuan_skill.name = "manyuan"
table.insert(sgs.ai_skills, manyuan_skill)
manyuan_skill.getTurnUseCard = function(self)
	if self.player:usedTimes("#manyuan") >= 1 or self.player:isKongcheng() then return end
	return sgs.Card_Parse("#manyuan:.:")
end
sgs.ai_skill_use_func["#manyuan"] = function(card, use, self)
	local N = self.player:getHandcardNum()
	local friends = sgs.SPlayerList()
	local target
	for _, vic in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if N == 1 and self:isEnemy(vic) and (((self:isWeak(vic) or self:willSkipPlayPhase(vic)) and vic:getMaxCards() > 0 and vic:getMaxCards() <= 2) or (self:needKongcheng(vic) and vic:isKongcheng())) then
			target = vic
			break
		end
		if self:isFriend(vic) and not (self:needKongcheng(vic) and vic:isKongcheng()) and not hasManjuanEffect(vic) then
			friends:append(vic)
		end
	end
	if target then
		if use.to then
			use.to:append(target)
		end
		card_str = "#manyuan:.:->"..target:objectName()
		use.card = sgs.Card_Parse(card_str)
	elseif not friends:isEmpty() then
		friends = sgs.QList2Table(friends)
		local target = self:findPlayerToDraw2(false, N, friends, true, true, false)
		if target then
			if use.to then
				use.to:append(target)
			end
			card_str = "#manyuan:.:->"..target:objectName()
			use.card = sgs.Card_Parse(card_str)
		end
	end
end

sgs.ai_use_value["manyuan"] = 1 --卡牌使用价值
sgs.ai_use_priority["manyuan"] = 1.29 --卡牌使用优先级

sgs.ai_card_intention.manyuan = function(self, card, from, tos)
    local to = tos[1]
    local intention = -10
    if from:getHandcardNum() == 1 or hasManjuanEffect(to) or (self:needKongcheng(to) and to:isKongcheng()) or self:willSkipPlayPhase(to) then
        intention = 0
    end
    sgs.updateIntention(from, to, intention)
end

sgs.ai_need_damaged.manyuan = function (self, attacker, player)
	if self.player:getHp() == 1 and self.player:getMaxHp() - self.player:getHp() + 1 >= 2 and self:getAllPeachNum() > 0 then
		return true
	end
	return false
end




--嘲讽
sgs.ai_chaofeng.beishanghuahuo_xuanlancuantianhou = 0

--------------------------------------------------
--自荐
--------------------------------------------------

sgs.ai_skill_playerchosen.zijian = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	local N = self.player:getMark("zijian_AI")
	local target = self:findPlayerToDraw2(false, N, targetlist, false, true, false)
	if target then
		return target
	end
end

sgs.ai_playerchosen_intention.zijian = function(self, from, to)
    local intention = -10
    if hasManjuanEffect(to) or (self:needKongcheng(to) and to:isKongcheng()) or self:willSkipPlayPhase(to) then
        intention = 0
    end
    sgs.updateIntention(from, to, intention)
end

--------------------------------------------------
--惊心
--------------------------------------------------

sgs.ai_skill_invoke.jingxin = function(self, data)
	local objname = string.sub(data:toString(), 8, -1)	--截取choice:后面的部分(即player:objectName())
	local from = findPlayerByFlag(self.room, "jingxin_usefrom_AI")
	if from and not self:isFriend(from) and self.player:getHp() >= 3 then	--状态好无脑发动，卖状态换牌
		return true
	end
	if (self.player:hasArmorEffect("vine") and (objname == "slash" or objname == "savage_assault" or objname == "archery_attack")) or (self.player:hasArmorEffect("renwang_shield") and (objname == "slash" or objname == "fire_slash" or objname == "thunder_slash")) then
		return not self:isWeak()
	end
	if (objname == "fire_slash" or objname == "thunder_slash" or objname == "fire_attack") and from and self:isFriend(from) then
		return false
	end
	if (objname == "fire_slash" or objname == "thunder_slash" or objname == "slash" or objname == "archery_attack") then	--需要出闪
		return self:getCardsNum("Jink") == 0 and not (objname == "archery_attack" and self:getCardsNum("Nullification") == 0)
	end
	if (objname == "duel" or objname == "savage_assault") then	--需要出杀
		return self:getCardsNum("Slash")+self:getCardsNum("Nullification") == 0
	end
	if (objname == "snatch" or (objname == "fire_attack" and self:isWeak())) then	--需要无懈
		return self:getCardsNum("Nullification") == 0
	end
	return false
end




--嘲讽
sgs.ai_chaofeng.leidi_haizaomao = 1

--------------------------------------------------
--沐光
--------------------------------------------------
--[[
sgs.ai_skill_invoke["muguang"] = function(self, data)	--yun
	local target = data:toPlayer()
	if target and self:canDraw(target) then
		if self:isFriend(target) then
			sgs.updateIntention(self.player, target, -10)
			return true
		else
			sgs.updateIntention(self.player, target, 10)
		end
	end
	return false
end

sgs.ai_choicemade_filter.skillInvoke.muguang = function(self, player, promptlist)	--yun
	local target = self.room:getCurrent()
	if target and self:canDraw(target, player) then
		if promptlist[#promptlist] == "yes" then
			sgs.updateIntention(player, target, -10) 
		elseif promptlist[#promptlist] == "no" then
			sgs.updateIntention(player, target, 10)
		end
	end
end]]

sgs.ai_skill_cardask["@muguang_give"] = function(self, data, pattern, target)
	local to = data:toPlayer()
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByCardNeed(cards, false)
	if self:getOverflow() > 0 or not self:isWeak() then		--自己不濒死或溢出就给最不需要的手牌
		for _, card in ipairs(cards) do
			if (self:canDraw(to) and self:isFriend(to)) or (not self:canDraw(to) and self:isEnemy(to)) then
				return "$" .. card:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_cardneed.muguang = function(to, card, self)	--需要自己留桃
	return card:isKindOf("Peach")
end

--------------------------------------------------
--摸鱼
--------------------------------------------------

local moyu_skill={}
moyu_skill.name="moyu"
table.insert(sgs.ai_skills,moyu_skill)
moyu_skill.getTurnUseCard=function(self,inclusive)
	if self.player:usedTimes("#moyu") < 1 and not self.player:isKongcheng() then
		local use_moyu = false
		
		local average_use_value = 0
		for _, cd in sgs.qlist(self.player:getHandcards()) do
			average_use_value = average_use_value + self:getUseValue(cd)
		end
		average_use_value = average_use_value / self.player:getHandcardNum()
		
		if average_use_value < 6 and (self.player:getHandcardNum() <= 2 or self:getOverflow() <= 0) then
			return sgs.Card_Parse("#moyu:.:")
		end
	end
end

sgs.ai_skill_use_func["#moyu"] = function(card,use,self)
	if not use.isDummy then self:speak("moyu") end
	use.card = card
end

sgs.ai_use_priority["moyu"] = 0





--嘲讽
sgs.ai_chaofeng.xuehusang_zizaisuixin = 1

--------------------------------------------------
--狐言
--------------------------------------------------

local huyan_skill={}
huyan_skill.name="huyan"
table.insert(sgs.ai_skills,huyan_skill)
huyan_skill.getTurnUseCard = function(self, inclusive)
	if self.player:usedTimes("#huyan") < 1 and not self.player:isKongcheng() and not self:needBear(self.player, false, nil, "xindong") and self:needToThrowHandcard(self.player, 1) then
		return sgs.Card_Parse("#huyan:.:")
	end
end
sgs.ai_skill_use_func["#huyan"] = function(card, use, self)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	
	local targetlist = self.room:getOtherPlayers(self.player)
	targetlist = sgs.QList2Table(targetlist)
	self:sort(targetlist, "defense")
	
	for _, card in ipairs(cards) do
		local class_name = card:getClassName()
		if class_name == "FireSlash" or class_name == "ThunderSlash" then
			class_name = "Slash"
		end
		for _, vic in ipairs(targetlist) do
			if SkillCanTarget(vic, self.player, "huyan") and not vic:isKongcheng() and getCardsNum(class_name, vic, self.player, true) > 0 then
				if (self:isFriend(vic) and self:needToThrowHandcard(vic, 1, sgs.CardMoveReason_S_REASON_EXTRACTION)) or (self:isEnemy(vic) and not self:needToThrowHandcard(vic, 1, sgs.CardMoveReason_S_REASON_EXTRACTION)) then
					if use.to then
						use.to:append(vic)
					end
					card_str = "#huyan:"..card:getId()..":->"..vic:objectName()
					use.card = sgs.Card_Parse(card_str)
					return
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		local class_name = card:getClassName()
		if class_name == "FireSlash" or class_name == "ThunderSlash" then
			class_name = "Slash"
		end
		if class_name == "Jink" and not self:isWeak() then
			for _, vic in ipairs(targetlist) do
				if SkillCanTarget(vic, self.player, "huyan") and not vic:isKongcheng() and self:isEnemy(vic) and self:isWeak(vic) and (vic:getHandcardNum() - getCardsNum("VisibleCard", vic, self.player)) > 0 then
					if use.to then
						use.to:append(vic)
					end
					card_str = "#huyan:"..card:getId()..":->"..vic:objectName()
					use.card = sgs.Card_Parse(card_str)
					return
				end
			end
		end
	end
	
	if self:needToThrowHandcard(self.player, 1) then
		for _, vic in ipairs(targetlist) do
			if (self:isFriend(vic) and self:needToThrowHandcard(vic, 1, sgs.CardMoveReason_S_REASON_EXTRACTION)) or (self:isEnemy(vic) and not self:needToThrowHandcard(vic, 1, sgs.CardMoveReason_S_REASON_EXTRACTION)) then
				local cards = self.player:getCards("h")
				cards = sgs.QList2Table(cards)
				local ids = dimeng_discard(self, 1, cards)
				if use.to then
					use.to:append(vic)
				end
				card_str = "#huyan:"..ids[1]..":->"..vic:objectName()
				use.card = sgs.Card_Parse(card_str)
				return
			end
		end
	end
end

sgs.ai_use_priority["huyan"] = sgs.ai_use_priority.Slash + 0.2

sgs.ai_card_intention.huyan = function(self, card, from, tos)
	if #tos > 0 then
		for _,to in ipairs(tos) do
			if not self:needToThrowHandcard(to, 1, sgs.CardMoveReason_S_REASON_EXTRACTION) then
				sgs.updateIntention(from, to, 10)
			end
		end
	end
	return 0
end

--------------------------------------------------
--心动
--------------------------------------------------

sgs.ai_skill_use["@@xindong"] = function(self, prompt, method)
	local targets = {}
	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:isFemale() and SkillCanTarget(p, self.player, "xindong") then
			table.insert(targets, p)
		end
	end
	local target = self:findPlayerToDamage(1, self.player, sgs.DamageStruct_Normal, targets, false, 0, false)
	if target then
		return "#xindong:.:->"..target:objectName()
	end
end

sgs.ai_playerchosen_intention.xindong = function(self, from, to)
	local intention = 50
	if self:isFriend(from, to) and table.contains(self:findPlayerToDamage(1, from, sgs.DamageStruct_Normal, {to}, false, 0, true), to) then
		intention = 0
	elseif self:needToLoseHp(to) or to:hasSkills(sgs.masochism_skill) then
		intention = 0
	end
	sgs.updateIntention(from, to, intention)
end




--嘲讽
sgs.ai_chaofeng.youzi_yuko = 1

--------------------------------------------------
--渊回
--------------------------------------------------

sgs.ai_skill_playerchosen.yuanhui = function(self, targetlist)
	targetlist = sgs.QList2Table(targetlist)
	self:sort(targetlist, "handcard")
	
	return self:findPlayerToDamage(1, self.player, sgs.DamageStruct_Normal, targetlist, false, 0, false)
end

--造成伤害自带仇恨值，不用写

sgs.ai_cardneed.yuanhui = function(to, card, self)	--需要黑杀
	return (card:isKindOf("Slash") and card:isBlack())
end



--嘲讽
sgs.ai_chaofeng.mutangchun_recorder = 0

--------------------------------------------------
--纪实
--------------------------------------------------

sgs.ai_skill_askforag["jishi_M"] = function(self, card_ids)
	if self:canDraw(self.player) then
		return self:askForAG(card_ids, false, "default")
	else
		return nil
	end
end

--------------------------------------------------
--同我
--------------------------------------------------

sgs.ai_skill_invoke.tongwo = function(self, data)
	if self:isWeak(self.player, true) then
		return true
	else
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			if self:isWeak(enemy) and self:canSlash(enemy) and (self:getCardsNum("Slash") > 0 or enemy:getHp() == 1) then
				has_weak_enemy = true
			end
		end
		return has_weak_enemy
	end
end



--嘲讽
sgs.ai_chaofeng.shaye_rougumeisheng = 0

--------------------------------------------------
--焚心
--------------------------------------------------

sgs.ai_view_as.fenxin_S = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand and card:isRed() and not card:isKindOf("Peach") and not card:hasFlag("using") then
		return ("fire_slash:fenxin_S[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local fenxin_S_skill = {}
fenxin_S_skill.name = "fenxin_S"
table.insert(sgs.ai_skills, fenxin_S_skill)
fenxin_S_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	local red_card
	self:sortByUseValue(cards, true)

	local useAll = false
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and not enemy:hasArmorEffect("EightDiagram") and self.player:distanceTo(enemy) <= self.player:getAttackRange() and self:isWeak(enemy)
			and getCardsNum("Jink", enemy, self.player) + getCardsNum("Peach", enemy, self.player) + getCardsNum("Analeptic", enemy, self.player) == 0 then
			useAll = true
			break
		end
	end

	local disCrossbow = false
	if self:getCardsNum("Slash") < 2 or self.player:hasSkill("paoxiao") then
		disCrossbow = true
	end

	local nuzhan_equip = false
	local nuzhan_equip_e = false
	self:sort(self.enemies, "defense")
	if self.player:hasSkill("nuzhan") then
		for _, enemy in ipairs(self.enemies) do
			if  not enemy:hasArmorEffect("EightDiagram") and self.player:distanceTo(enemy) <= self.player:getAttackRange()
			and getCardsNum("Jink", enemy) < 1 then
				nuzhan_equip_e = true
				break
			end
		end
		for _, card in ipairs(cards) do
			if card:isRed() and card:isKindOf("TrickCard") and nuzhan_equip_e then
				nuzhan_equip = true
				break
			end
		end
	end

	local nuzhan_trick = false
	local nuzhan_trick_e = false
	self:sort(self.enemies, "defense")
	if self.player:hasSkill("nuzhan") and not self.player:hasFlag("hasUsedSlash") and self:getCardsNum("Slash") > 1 then
		for _, enemy in ipairs(self.enemies) do
			if  not enemy:hasArmorEffect("EightDiagram") and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
				nuzhan_trick_e = true
				break
			end
		end
		for _, card in ipairs(cards) do
			if card:isRed() and card:isKindOf("TrickCard") and nuzhan_trick_e then
				nuzhan_trick = true
				break
			end
		end
	end

	for _, card in ipairs(cards) do
		if card:isRed() and not card:isKindOf("Slash") and not (nuzhan_equip or nuzhan_trick)
			and (not isCard("Peach", card, self.player) and not isCard("ExNihilo", card, self.player) and not useAll)
			and (not isCard("Crossbow", card, self.player) and not disCrossbow)
			and (self:getUseValue(card) < sgs.ai_use_value.Slash or inclusive or sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, sgs.Sanguosha:cloneCard("slash")) > 0) then
			red_card = card
			break
		end
	end

	if nuzhan_equip then
		for _, card in ipairs(cards) do
			if card:isRed() and card:isKindOf("EquipCard") then
				red_card = card
				break
			end
		end
	end

	if nuzhan_trick then
		for _, card in ipairs(cards) do
			if card:isRed() and card:isKindOf("TrickCard")then
				red_card = card
				break
			end
		end
	end

	if red_card then
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("fire_slash:fenxin_S[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)
		return slash
	end
end

function sgs.ai_cardneed.fenxin_S(to, card)
	return to:getHandcardNum() < 3 and card:isRed()
end



--嘲讽
sgs.ai_chaofeng.laila_xuelie = -1

--------------------------------------------------
--忆狩
--------------------------------------------------

sgs.ai_skill_invoke.yishou = function(self,data)
	return not self:isWeak()
end

sgs.ai_skill_discard["yishou"] = function(self, discard_num, min_num, optional, include_equip)	--yun
	local cards = sgs.QList2Table(self.player:getCards("he"))
	local to_discard = {}
	if not (self:willSkipPlayPhase() or self:willSkipDrawPhase()) then		--不被控就直接用，防御是什么能吃吗
		to_discard = dimeng_discard(self, 1, cards)
	end
	return to_discard
end

--------------------------------------------------
--祭血
--------------------------------------------------

local jixue_skill={}
jixue_skill.name="jixue"
table.insert(sgs.ai_skills,jixue_skill)
jixue_skill.getTurnUseCard=function(self,inclusive)
	if self.player:getMark("@jixue") > 0 and self.player:canDiscard(self.player, "he") then
		local red_count = 0
		for _,card in sgs.qlist(self.player:getCards("he")) do
			if card:isRed() and not self.player:isJilei(card) then
				red_count = red_count + 1
			end
		end
		if (red_count > 0 and self:isWeak()) or (red_count > 2 and self:getOverflow() > 2) then
			return sgs.Card_Parse("#jixue:.:")
		end
	end
end

sgs.ai_skill_use_func["#jixue"] = function(card,use,self)
	if not use.isDummy then self:speak("jixue") end
	use.card = card
end

sgs.ai_use_priority["jixue"] = 3.1	--酒为3.0杀为2.6

sgs.ai_skill_choice.jixue = function(self, choices)
	if self:isWeak() and self.player:isWounded() then
		return "jixue_recover"
	end
	return "jixue_draw"
end



--嘲讽
sgs.ai_chaofeng.xingxi_tianjiliuxing = -5

--------------------------------------------------
--星耀
--------------------------------------------------

sgs.ai_skill_invoke.xingyao = function(self,data)
	local use = data:toCardUse()
	for _, p in sgs.qlist(use.to) do
		if self:isEnemy(p) then
			return true
		end
	end
	return false
end

sgs.ai_choicemade_filter.skillInvoke.xingyao = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = findPlayerByObjectName(self.room, promptlist[#promptlist - 1])
		if target then sgs.updateIntention(player, target, 10) end
	end
end

--------------------------------------------------
--余光
--------------------------------------------------

sgs.ai_skill_playerchosen.yuguang = function(self, targets)
	return self:findPlayerToDiscard("he", false, true, targets, false)
end




--嘲讽
sgs.ai_chaofeng.xiaomao_lairikeqi = 1

--------------------------------------------------
--辨识
--------------------------------------------------

local bianshi_skill={}
bianshi_skill.name="bianshi"
table.insert(sgs.ai_skills,bianshi_skill)
bianshi_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum() < 4 + self.player:getEquips():length() and not self.player:isKongcheng() and (not self.player:hasUsed("#bianshicard")) then
		return sgs.Card_Parse("#bianshicard:.:")
	end
end

sgs.ai_skill_use_func["#bianshicard"] = function(card, use, self)
	use.card = card
end

sgs.ai_use_value["bianshiCard"] = 2
sgs.ai_use_priority["bianshiCard"] = 0

--------------------------------------------------
--成长
--------------------------------------------------

sgs.ai_skill_invoke.chengzhang = function(self, data)
	return true
end




--嘲讽
sgs.ai_chaofeng.shayin_linglongyemo = 0

--------------------------------------------------
--魔音
--------------------------------------------------

local moyin_skill = {}
moyin_skill.name = "moyin"
table.insert(sgs.ai_skills, moyin_skill)
moyin_skill.getTurnUseCard = function(self)
	if self.player:usedTimes("#moyin") >= 1 then return end
	return sgs.Card_Parse("#moyin:.:")
end
sgs.ai_skill_use_func["#moyin"] = function(card, use, self)
	local targets = sgs.SPlayerList()
	for _, vic in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if SkillCanTarget(vic, self.player, "moyin") then
			targets:append(vic)
		end
	end
	if not targets:isEmpty() then
		local first, second, third, forth
		
		targets = sgs.QList2Table(targets)
		self:sort(targets, "defense")
		for _, to in ipairs(targets) do
			if self:isFriend(to) and to:isKongcheng() and self:canDraw(to) then		--空城队友
				first = to
				break
			elseif self:isEnemy(to) and to:isKongcheng() and self:needKongcheng(to) then	--需要空城的敌人（夏鹤仪震怒）
				first = to
				break
			elseif self:isEnemy(to) and getCardsNum("Jink", to, self.player) > 0 then		--有明闪的敌人
				second = to
			elseif self:isFriend(to) and getCardsNum("Jink", to, self.player) == 0 and self:canDraw(to) then	--明没有闪的队友
				third = to
			elseif self:isEnemy(to) and self:isWeak(to) and to:getHandcardNum() - getCardsNum("VisibleCard", to, self.player) > 0 then	--虚弱且有未知牌的敌人
				forth = to
			end
		end
		
		local target = first or second or third or forth	--or连接多个操作数时，表达式的返回值就是从左到右第一个不为假的值，若全部操作数值都为假，则表达式的返回值为最后一个操作数
		if target then
			if use.to then
				use.to:append(target)
			end
			card_str = "#moyin:.:->"..target:objectName()
			use.card = sgs.Card_Parse(card_str)
		end
	end
end

sgs.ai_use_value["moyin"] = 1 --卡牌使用价值
sgs.ai_use_priority["moyin"] = sgs.ai_use_priority.Slash + 0.21 --卡牌使用优先级

sgs.ai_card_intention.moyin = function(self, card, from, tos)
    local to = tos[1]
    local intention = 0
    if to:isKongcheng() then
		if self:canDraw(to) then
			intention = -10
		else
			intention = 10
		end
	elseif to:getHandcardNum() > 3 or getCardsNum("Jink", to, from) > 0 then
		if not self:needKongcheng(to) then
			intention = 10
		end
	elseif getCardsNum("Jink", to, from) == 0 then
		if self:canDraw(to) then
			intention = -10
		end
    end
    sgs.updateIntention(from, to, intention)
end

sgs.ai_skill_cardask["@moyin_discard"] = function(self, data, pattern, target)
	local jink_list = {}
	for _,cd in sgs.qlist(self.player:getHandcards()) do
		if cd:isKindOf("Jink") then
			table.insert(jink_list, cd)
		end
	end
	self:sortByKeepValue(jink_list)
	return "$"..jink_list[1]:getEffectiveId()
end




--嘲讽
sgs.ai_chaofeng.ciyuanjiang_mengxinyindaoyuan = 0

--------------------------------------------------
--次元共振
--------------------------------------------------

sgs.ai_skill_invoke.ciyuangongzhen = function(self, data)
	local room = self.player:getRoom()
	local nearname1 = ""
	local nearname2 = ""
	local other = nil
	for _,p in sgs.qlist(room:getAllPlayers()) do
		if p:getSeat() == math.mod(self.player:getSeat(),room:alivePlayerCount())+1 then
			nearname1 = p:objectName()
			other = p
		end
		if self.player:getSeat() == math.mod(p:getSeat(),room:alivePlayerCount())+1 then
			nearname1 = p:objectName()
		end
	end
	if nearname1 == nearname2 then
		if other:getHp() > self.player:getHp() and not self:isWeak() then
			return false
		end
	end
	return true
end

sgs.ai_skill_choice.ciyuangongzhen = function(self, choices)
	local cards = self.player:getHandcards()
	local friendNum = 0
	local friendNude = 0
	local enemyNum = 0
	local enemyNude = 0
	local unknownNum = 0
	local draw = false
	local discard = false
	local other = nil
	
	for _,p in sgs.qlist(self.room:getAllPlayers()) do
		if p:getMark("ciyuangongzhen") > 0 then
			if self:isFriend(p) then
				friendNum = friendNum + 1
				if p:isNude() then
					friendNude = friendNude + 1
				end
			elseif self:isEnemy(p) then
				enemyNum = enemyNum + 1
				other = p
				if p:isNude() then
					enemyNude = enemyNude + 1
				end
				if self:getDangerousCard(to) or self:getValuableCard(to) then	--敌人有高价值牌，八说了开拆
					return "ciyuangongzhen_discard"
				end
			else
				unknownNum = unknownNum + 1
			end
		end
	end
	local total = friendNum + enemyNum + unknownNum
	
	if total == 3 then
		if friendNum > enemyNum then
			draw = true
		elseif (friendNum - friendNude) < (enemyNum - enemyNude) then
			discard = true
		else
			if self.player:getHandcardNum() >= self.player:getHp() and self.player:getHp() >= 2 then
				discard = true
			else
				draw = true
			end
		end
	elseif total == 2 then
		if enemyNum == 0 then
			draw = true
		else
			if self.player:isNude() and not other:isNude() then
				return "ciyuangongzhen_discard"
			end
			if self.player:getHp() > other:getHp() and self.player:getHandcardNum() >= 2 and not other:isNude() then
				discard = true
			elseif self.player:getHp() > other:getHp() and (self.player:getHandcardNum() < 2 or other:isNude()) then
				draw = true
			elseif self.player:getHp() == other:getHp() then
				if self.player:getHandcardNum() >= 2 and not other:isNude() then
					discard = true
				else
					draw = true
				end
			elseif other:getHp() > self.player:getHp() and self:isWeak() then
				draw = true
			end
		end
	else
		draw = true
	end
	for _,p in sgs.qlist(self.room:getAllPlayers()) do
		if p:getMark("ciyuangongzhen") > 0 then
			if self:isFriend(p) and not self:canDraw(p) then	--队友不能摸就不选摸牌
				draw = false
			end
		end
	end
	if draw then
		return "ciyuangongzhen_drawcard"
	end
	if discard then
		return "ciyuangongzhen_discard"
	end
	return "cancel"
end



--嘲讽
sgs.ai_chaofeng.kuji_chaoyongyuge = 1

--------------------------------------------------
--低吟
--------------------------------------------------

local function get_handcard_suit(cards)
	if #cards == 0 then return sgs.Card_NoSuit end
	if #cards == 1 then return cards[1]:getSuit() end
	local black = false
	if cards[1]:isBlack() then black = true end
	for _, c in ipairs(cards) do
		if black ~= c:isBlack() then return sgs.Card_NoSuit end
	end
	return black and sgs.Card_NoSuitBlack or sgs.Card_NoSuitRed
end

local diyin_skill = {}
diyin_skill.name = "diyin"
table.insert(sgs.ai_skills, diyin_skill)
diyin_skill.getTurnUseCard = function(self, inclusive)
	sgs.ai_use_priority["diyin"] = 1.5
	if self.player:getMark("diyin_used") > 0 or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	local suit = get_handcard_suit(cards)
	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad = 0, 0
	local caocao = self.room:findPlayerBySkillName("jianxiong")
	local diyintrick = "savage_assault|archery_attack|duel|fire_attack"
	local diyintricks = diyintrick:split("|")
	local aoe_available, ge_available, ex_available = true, true, true
	local dl_available, fa_available = true, true
	for i = 1, #diyintricks do
		local forbiden = diyintricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, suit)
		if self.player:isCardLimited(forbid, sgs.Card_MethodUse, true) or not forbid:isAvailable(self.player) then
			if forbid:isKindOf("AOE") then aoe_available = false end
			if forbid:isKindOf("GlobalEffect") then ge_available = false end
			if forbid:isKindOf("ExNihilo") then ex_available = false end
			if forbid:isKindOf("Duel") then dl_available = false end
			if forbid:isKindOf("FireAttack") then fa_available = false end
		end
	end
	if self.player:getMark("diyin_used") > 0 then return end
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10 / friend:getHp()
			if friend:isLord() then good = good + 10 / friend:getHp() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10 / enemy:getHp()
			if enemy:isLord() then
				bad = bad + 10 / enemy:getHp()
			end
		end
	end

	local can_use = false
	local total_keep_value = 0
	for _, card in ipairs(cards) do
		table.insert(allcard, card:getId())
		total_keep_value = total_keep_value + self:getKeepValue(card)
		if self:willUse(self.player, card) then
			can_use = true
		end
	end

	if #allcard > 1 then sgs.ai_use_priority["diyin"] = 0 end
	if self.player:getHandcardNum() == 1 or (self.player:getHandcardNum() <= 3 and not can_use) then
		if aoe_available then
			for i = 1, #aoenames do
				local newdiyin = aoenames[i]
				aoe = sgs.Sanguosha:cloneCard(newdiyin)
				if self:getAoeValue(aoe) > 0 then
					--local parsed_card = sgs.Card_Parse("#diyin:" .. table.concat(allcard, "+") .. ":" .. newdiyin)
					local parsed_card = sgs.Card_Parse(newdiyin .. ":diyin[no_suit:0]=".. table.concat(allcard, "+"))
					return parsed_card
				end
			end
		end
		
		local duel = sgs.Sanguosha:cloneCard("duel", sgs.Card_NoSuit, 0)
		for _,card in sgs.qlist(self.player:getHandcards()) do
			duel:addSubcard(card)
		end
		duel:setSkillName("diyin")
		if self:willUse(self.player, duel, false, false, true) and dl_available then
			local parsed_card = sgs.Card_Parse("duel:diyin[no_suit:0]=".. table.concat(allcard, "+"))
			return parsed_card
		end
		
		if total_keep_value < 6 and fa_available then
			local parsed_card = sgs.Card_Parse("fire_attack:diyin[no_suit:0]=".. table.concat(allcard, "+"))
			return parsed_card
		end
	end

	if aoe_available then
		for i = 1, #aoenames do
			local newdiyin = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newdiyin)
			if self:getAoeValue(aoe) > -5 and caocao and self:isFriend(caocao) and caocao:getHp() > 1 and not self:willSkipPlayPhase(caocao)
				and not self.player:hasSkill("jueqing") and self:aoeIsEffective(aoe, caocao, self.player) then
				--local parsed_card = sgs.Card_Parse("#diyin:" .. table.concat(allcard, "+") .. ":" .. newdiyin)
				local parsed_card = sgs.Card_Parse(newdiyin .. ":diyin[no_suit:0]=".. table.concat(allcard, "+"))
				return parsed_card
			end
		end
	end
end

sgs.ai_skill_use_func["#diyin"] = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[4]
	local diyincard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	diyincard:setSkillName("diyin")
	self:useTrickCard(diyincard, use)
	if use.card then
		for _, acard in sgs.qlist(self.player:getHandcards()) do
			if isCard("Peach", acard, self.player) and self.player:getHandcardNum() > 1 and self.player:isWounded()
				and not self:needToLoseHp(self.player) then
					use.card = acard
					return
			end
		end
		use.card = card
	end
end

sgs.ai_use_priority["diyin"] = 1.5




--嘲讽
sgs.ai_chaofeng.jiuma_hanshixianggong = 0

--------------------------------------------------
--制冷
--------------------------------------------------

sgs.ai_skill_discard["zhileng"] = function(self, discard_num, min_num, optional, include_equip)	--yun
	local need_avoid = false
	local data = self.player:getTag("zhileng")
	local damage = data:toDamage()
	if damage and self:damageIsEffective_(damage) and not self:getDamagedEffects(damage.to, damage.from, damage.card and damage.card:isKindOf("Slash"))
		and not self:needToLoseHp(damage.to, damage.from, damage.card and damage.card:isKindOf("Slash")) then
		need_avoid = true
	end
	
	if need_avoid then
		local cards = sgs.QList2Table(self.player:getCards("he"))
		local to_discard = {}
		to_discard = dimeng_discard(self, 2, cards)
		return to_discard
	end
	return nil
end
