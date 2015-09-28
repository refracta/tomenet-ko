-- Tables with descriptive text about the races, to be displayed
-- during character creation process. - C. Blue

function get_race_diz(r_title, l)
    local i
    i = 0
    while __races_diz[i * 2 + 1] ~= nil do
	if __races_diz[i * 2 + 1] == r_title then
	    if __races_diz[i * 2 + 2][l + 1] == nil then
		return ""
	    else
		return __races_diz[i * 2 + 2][l + 1]
	    end
	end
	i = i + 1
    end
    return ""
end

__races_diz = {

--  "12345678901234567890123456789012345678901234567890",

"Human",
    {"\255uHumans\255U are allrounders with no special weaknesses ",
    "or strengths. They have no experience penalties   ",
    "and tend to go up in levels faster than most other",
    "races.",
    "They get a bonus on Climbing and Interception.",
    "", "", "", "", "", "", "",},

"Half-Elf",
    {"\255uHalf-elves\255U tend to be smarter and faster than",
    "humans, but not as strong. They are slightly",
    "better at searching, disarming, saving throws,",
    "stealth, bows and magic, but they are not as good",
    "at hand weapons. Half-elves resist light intrinsi-",
    "cally. They get a bonus on Bow-Mastery and on",
    "Interception.",
    "", "", "", "", "",},

"Elf",
    {"\255uElves\255U resist light intrinsically and see invisible",
    "creatures. They are better magicians than humans,",
    "but not as good at fighting. They tend to be wiser",
    "and smarter than humans or half-elves and good at",
    "searching, disarming, perception, stealth, bows/",
    "throwing and magic.",
    "They get a bonus on Bow-Mastery, Magic, Stealth,",
    "Dodging, Calmness and Sneakiness.",
    "", "", "", "",},

"Hobbit",
    {"\255uHobbits\255U have infra-vision, resist black breath",
    "somewhat, have their dexterity sustained and get",
    "a dexterity bonus for not wearing shoes.",
    "They make excellent rogues (they prefer to be",
    "called burglars), but might want to train 'Health'",
    "skill to make up for having lower hit points than",
    "humans. Their strengths are bows/throwing, saving",
    "throw, searching, disarming, perception, stealth.",
    "They receive a bonus on Sling-Mastery, Sneakiness,",
    "Dodging, and especially Stealth.",
    "", "",},

"Gnome",
    {"\255uGnomes\255U live in the earth, in burrow-like homes.",
    "They make excellent spellcasters and resist",
    "paralysis intrinsically. They are good at stealth,",
    "searching, perception and disarming. Weaker than",
    "humans, they aren't very good at close combat.",
    "Gnomes have infra-vision and receive a bonus on",
    "the skills Crossbow-Mastery, Magic, and a large",
    "bonus on Dodging and Digging.",
    "", "", "", "",},

"Dwarf",
    {"\255uDwarves\255U are headstrong miners and fighters of",
    "legend. Strong and wise, they are less intelli-",
    "gent than humans though but can never be blinded",
    "and learn to pass even the highest mountain",
    "regions at level 30. Since they live underground",
    "they have infra-vision, their toughness makes for",
    "a good saving throw and they are good with axes.",
    "They do have one big drawback, though. They are",
    "loudmouthed and proud, singing in loud voices,",
    "arguing with themselves for no good reason, and",
    "screaming out challenges to nearby foes. In other",
    "words, dwarves have miserable stealth.",},

"Half-Orc",
    {"\255uHalf-Orcs\255U resist darkness because of their",
    "preference to living underground. They make great",
    "warriors and rogues, but aren't good at magic.",
    "Their stealth isn't great either, and they are bad",
    "at disarming, searching and perception. Half-orcs",
    "are quite ugly and tend to pay more for goods in",
    "town. Their constitution however is pretty good.",
    "They receive a slight bonus on Backstabbing and a",
    "large bonus on Weaponmastery, as well as a bonus",
    "on Necromancy. They receive penalties on Magic and",
    "Sorcery.",
    "",},

"Half-Troll",
    {"\255uHalf-trolls\255U have their strength sustained and are",
    "very strong but also stupid and always hungry.",
    "They are so ugly that a half-orc grimaces in their",
    "presence. Half-trolls are bad at searching, disar-",
    "ming, perception and stealth, get a penalty on",
    "Critical-Strike and large penalties on Archery,",
    "Magic and Sorcery, but receive a bonus on Health",
    "and Blunt-Mastery.",
    "", "", "", "",},

"Dunadan",
    {"\255uDunadain\255U are a hardy race of men from the west.",
    "This elder race surpasses human abilities in every",
    "field, especially constitution. However, being men",
    "of the world, very little is new to them and expe-",
    "rience is harder for them to gain. Dunadain have",
    "their constitution sustained and receive boni on",
    "Combat, Archery and Interception skills and a",
    "small bonus to Magic.",
    "", "", "", "",},

"High-Elf",
    {"\255uHigh-elves\255U can see the invisible and resist light",
    "and the flow of time intrinsically. They are a",
    "race of immortal beings, dating far back to when",
    "Arda was young, masters of skills, strong and",
    "intelligent, although their wisdom is sometimes",
    "suspect. They do very well in all professions but",
    "there are few things they haven't seen yet and so",
    "experience is hard for them to gain.",
    "High-elves receive boni on Bow-Mastery, Magic,",
    "Stealth, Dodging, and a large bonus on Calmness.",
    "", "",},

"Yeek",
    {"\255uYeeks\255U, being weak and losing many childhood",
    "fights, have intrinsic feather falling.",
    "Often having to escape from precarious situations,",
    "they are able to pass through thick forest easily.",
    "Yeeks are the least powerful of all races and",
    "while they do not have particular strengths or",
    "weaknesses similar as humans, they suffer disad-",
    "vantages in all fields. However, to compensate",
    "they learn very quickly, gaining levels fastest",
    "of all the races.",
    "", "",},

"Goblin",
    {"\255uGoblins\255U have a decidedly mixed reputation.",
    "They are almost always grinning, and more often",
    "than not you cannot tell whether or not they are",
    "eyeing you like a side of meat. They are as sturdy",
    "as dwarves and quite agile, making them good",
    "fighters and rogues. Being creatures of the night",
    "Goblins have natural resistance to darkness.",
    "They receive boni on Boomerang-Mastery and",
    "Stealing.",
    "", "", "",},

"Ent",
    {"\255uEnts\255U are a powerful race of tree-like beings,",
    "dating from the beginning of the world. They are",
    "slow but very strong and tough, and have to feed",
    "rarely, which they can do simply by resting on",
    "earthen ground, apart from using magical means.",
    "Consisting of wood, they are susceptible to fire,",
    "but resist water. As they grow stronger, they",
    "receive various types of telepathic vision.",
    "Ents get boni to Health, and large boni on Earth,",
    "Water and Calmness. They receive penalties on Axe-",
    "mastery, Sword-mastery and a large penalty on the",
    "magic Fire school.",},

"Draconian",
    {"\255uDraconians\255U are half-breeds of dragon and men who",
    "surpass humans in all ways, except they are really",
    "bad at moving stealthily. Draconians are able to",
    "consume firestones to boost their powers and they",
    "have intrinsic feather falling and gain telepathic",
    "vision of dragons at level 5 and generic short",
    "range telepathy at level 6 which expands further",
    "as they get stronger. At 30 they learn how to",
    "levitate. Experience is very hard for them to gain",
    "and they need to eat often. They receive boni on",
    "Attunement, Polearm-Mastery and Magic Devices.",
    "",},

"Kobold",
    {"\255uKobolds\255U are somewhat smallish dog-faced humanoids",
    "living in cave complexes. Their intelligence is",
    "somewhat suspect and they do not usually pick up",
    "careers as mages or other more elaborate profes-",
    "sions but they make decent shamans or druids.",
    "Regarding fighting they are somewhat better at",
    "archery than in close combat. They resist poison",
    "intrinsically, have infra-vision, extraordinarily",
    "sharp senses and can move very silently. They are",
    "also good at disarming traps. Kobolds receive a",
    "large bonus on Swimming, a bonus on Trapping",
    "and a slight bonus on Sneakiness.",},

"Dark-Elf",
    {"\255uDark-elves\255U dwell below the surface in huge caverns",
    "of their underground city, in a very peculiar",
    "society. The children are already drilled to",
    "become sworn enemies of most other races of the",
    "underground and also of the hated elves who live",
    "on the surface under the bright sun. Overthrowing",
    "rivals by murder and treachery are acknowledged as",
    "long as noone is left alive to bring up charges.",
    "Dark-elves make good fighters and mages, but also",
    "decent priests. They resist darkness intrinsically",
    "but are susceptible to light. They receive boni",
    "on Stealth, Dodging, Sneakiness, Backstabbing.",},

"Vampire",
    {"\255uVampires\255U are powerful undead who cannot move",
    "in the burning sunlight, and who feast on blood to",
    "sustain themselves. They resist darkness but are",
    "susceptible to light and certain life-supportive",
    "and undead-vanquishing magic. They do not need a",
    "light source, and resist time, nether, poison and",
    "life-force drain. They also have life-leeching",
    "melee attacks. They are unable to utilize Water",
    "or Nature magic and are bad at Light runecraft.",
    "Vampires hate blunt weapons or divination magic,",
    "but get boni on Mental Intrusion, Necromancy and",
    "auras, Nether runecraft and Traumaturgy.",},

"Maia",
    {"\255uMaiar\255U are lesser spirits associated with one",
    "or more Valar, the greater gods. They appear in",
    "either human or elven form, and often act as ",
    "powerful wizards to steer the fate of Arda's",
    "inhabiting races. They must kill either a candle-",
    "bearer or a darkling before reaching level 20 in",
    "order to get initiated to either corruption or",
    "enlightenment, bestowing upon the Maia demonic or",
    "angelic traits. Should they fail, or kill both",
    "then their physical form will be forced to leave",
    "this realm again. They have access to Astral magic",
    "and receive boni on Sneakiness and Stealth.",},
}
