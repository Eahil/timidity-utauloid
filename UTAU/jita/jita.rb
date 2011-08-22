

[
"a","i","u","e","o",
"ka","ki","ku","ke","ko",
"ga","gi","gu","ge","go",
"sa","si","su","se","so",
"za","zi","zu","ze","zo",
"ta","ti","tu","te","to",
"da","di","du","de","do",
"na","ni","nu","ne","no",
"ha","hi","hu","he","ho",
"ba","bi","bu","be","bo",
"pa","pi","pu","pe","po",
"va","vi","vu","ve","vo",
"ma","mi","mu","me","mo",
"ra","ri","ru","re","ro",
"la","li","lu","le","lo",
"ja","jo","ju","wa","wo",
"tsu","fu","shi",
"kja","kju","kjo",
"hja","hju","hjo",
"nja","nju","njo",
"mja","mju","mjo",
"mja","mju","mjo",
"sha","shu","sho",
"cha","chu","cho"
].each do|sy|
	f = File.open("#{sy}.fest", 'w');
	f.write "(voice_czech_dita)(utt.save.wave (SayText \"#{sy}\") \"#{sy}.wav\" 'riff)(exit)\n"
	f.close
end

# 

