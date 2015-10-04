-- handle the holy support school

HDELFEAR = add_spell {
	["name"] = 	"Remove Fear",
	["school"] = 	{SCHOOL_HSUPPORT},
	["am"] = 	0,
	["level"] = 	1,
	["mana"] = 	3,
	["mana_max"] = 	3,
	["fail"] = 	10,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			fire_ball(Ind, GF_REMFEAR_PLAYER, 0, get_level(Ind, HDELFEAR, 30 * 2), 4, " speaks some faithful words and you lose your fear.")
			set_afraid(Ind, 0)
			set_res_fear(Ind, get_level(Ind, HDELFEAR, 30))
			end,
	["info"] = 	function()
			return "dur "..get_level(Ind, HDELFEAR, 30)
			end,
	["desc"] = 	{
			"Removes fear from your heart for a while.",
			"***Automatically projecting***",
		}
}

HSANCTUARY_I = add_spell {
	["name"] = 	"Sanctuary I",
	["school"] = 	{SCHOOL_HSUPPORT},
	["am"] = 	75,
	["level"] = 	3,
	["mana"] = 	5,
	["mana_max"] = 	5,
	["fail"] = 	10,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
--			project(0 - Ind, get_level(Ind, HSANCTUARY, 10), player.wpos, player.py, player.px, (3 + get_level(Ind, HSANCTUARY, 30)) * 2, GF_OLD_SLEEP, 64+16+8, "mumbles softly")
			project(0 - Ind, get_level(Ind, HSANCTUARY, 10), player.wpos, player.py, player.px, 10 + get_level(Ind, HSANCTUARY, 80), GF_OLD_SLEEP, 64+16+8, "mumbles softly")
	end,
	["info"] = 	function()
			return "power "..(5 + get_level(Ind, HSANCTUARY, 80)).." rad "..get_level(Ind, HSANCTUARY, 10)
	end,
	["desc"] = 	{
			"Lets monsters next to you fall asleep",
	}
}
HSANCTUARY_II = add_spell {
	["name"] = 	"Sanctuary II",
	["school"] = 	{SCHOOL_HSUPPORT},
	["am"] = 	75,
	["level"] = 	23,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] = 	-5,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
--			project_los(Ind, GF_OLD_SLEEP, 3 + get_level(Ind, HSANCTUARY, 25), "mumbles softly")
			project_los(Ind, GF_OLD_SLEEP, 5 + get_level(Ind, HSANCTUARY, 80), "mumbles softly")
	end,
	["info"] = 	function()
			return "power "..(5 + get_level(Ind, HSANCTUARY, 80))
	end,
	["desc"] = 	{
			"Lets nearby monsters fall asleep",
	}
}

HSATISFYHUNGER = add_spell {
	["name"] = 	"Satisfy Hunger",
	["school"] = 	{SCHOOL_HSUPPORT},
	["am"]  =	75,
	["level"] = 	10,
	["mana"] = 	20,
	["mana_max"] = 	20,
	["fail"] = 	40,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			set_food(Ind, PY_FOOD_MAX - 1)
			fire_ball(Ind, GF_SATHUNGER_PLAYER, 0, 1, 3, "")
			end,
	["info"] = 	function()
			return ""
			end,
	["desc"] = 	{
			"Satisfies your hunger.",
			"***Automatically projecting***",
			}
}

HSENSE_I = add_spell {
	["name"] = 	"Sense Surroundings I",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	75,
	["level"] = 	20,
	["mana"] = 	20,
	["mana_max"] = 	20,
	["fail"] = 	25,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			map_area(Ind)
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] = 	{
			"Maps the dungeon around you.",
	}
}
HSENSE_II = add_spell {
	["name"] = 	"Sense Surroundings II",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	75,
	["level"] = 	40,
	["mana"] = 	80,
	["mana_max"] = 	80,
	["fail"] = 	5,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			wiz_lite_extra(Ind)
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] = 	{
			"Gives clairvoyance, mapping and lighting up the whole dungeon.",
	}
}

HDETECTEVIL = add_spell {
	["name"] = 	"Detect Evil",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	75,
	["level"] = 	3,
	["mana"] = 	3,
	["mana_max"] = 	3,
	["fail"] = 	15,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			detect_evil(Ind)
			end,
	["info"] = 	function()
			return ""
			end,
	["desc"] = 	{
			"Detects all nearby non-invisible creatures once and also lets",
			"you see invisible creatures for a while.",
			"At level 30 it lets you sense the presence creatures for a while.",
	}
}
HSENSEMON = add_spell {
	["name"] = 	"Sense Monsters",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	75,
	["level"] = 	33,
	["mana"] = 	15,
	["mana_max"] = 	15,
	["fail"] = 	15,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
			set_tim_esp(Ind, 10 + randint(10) + 30 + get_level(Ind, HSENSEMON, 20))
			end,
	["info"] = 	function()
			return ""
			end,
	["desc"] = 	{
			"Lets you sense the minds of monsters for a while.",
	}
}

HZEAL_I = add_spell {
	["name"] = 	"Zeal I",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	50,
	["level"] = 	32,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	5,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
		local d, p
		d = 9 + randint(5)
		p = 10
		set_zeal(Ind, p, d)
		fire_ball(Ind, GF_ZEAL_PLAYER, 0, (p * 4) / 3, 3, "")
	end,
	["info"] = 	function()
			return "dur 9+d5, "..(get_zeal_power() / 10).." EA"
	end,
	["desc"] = 	{
		"Increases your melee attacks per round by up to +3 for 9+d5 turns.",
		"***Automatically projecting (+1 EA)***",
	}
}
HZEAL_II = add_spell {
	["name"] = 	"Zeal II",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	50,
	["level"] = 	41,
	["mana"] = 	100,
	["mana_max"] = 	100,
	["fail"] = 	-10,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
		local d, p
		d = 9 + randint(5)
		p = 20
		set_zeal(Ind, p, d)
		fire_ball(Ind, GF_ZEAL_PLAYER, 0, (p * 4) / 3, 3, "")
	end,
	["info"] = 	function()
			return "dur 9+d5, "..(get_zeal_power() / 10).." EA"
	end,
	["desc"] = 	{
		"Increases your melee attacks per round by up to +3 for 9+d5 turns.",
		"***Automatically projecting (+1 EA)***",
	}
}
HZEAL_III = add_spell {
	["name"] = 	"Zeal III",
	["school"] = 	SCHOOL_HSUPPORT,
	["am"] = 	50,
	["level"] = 	50,
	["mana"] = 	150,
	["mana_max"] = 	150,
	["fail"] = 	-25,
	["stat"] = 	A_WIS,
	["spell"] = 	function()
		local d, p
		d = 9 + randint(5)
		p = 30
		set_zeal(Ind, p, d)
		fire_ball(Ind, GF_ZEAL_PLAYER, 0, (p * 4) / 3, 3, "")
	end,
	["info"] = 	function()
			return "dur 9+d5, "..(get_zeal_power() / 10).." EA"
	end,
	["desc"] = 	{
		"Increases your melee attacks per round by up to +3 for 9+d5 turns.",
		"***Automatically projecting (+2 EA) ***",
	}
}
