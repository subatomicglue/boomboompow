
all: universalIcons universalLaunchScreens

makeimage = rm -f $(2).png; cp $(1) $(2).png; sips -z $(4) $(3) $(2).png > /dev/null; echo "      <string>$(2)</string>"

makelaunch = rm -f $(2).png; cp $(1) $(2).png; sips -z $(4) $(3) $(2).png > /dev/null; echo "      <string>$(2)</string>"

iPhoneIcons:
	$(call makeimage,mantis-original.png,mantisicon512x512,512,512)
	$(call makeimage,mantis-original.png,iTunesArtwork,512,512)
	$(call makeimage,mantis-original.png,mantisicon1024x1024,1024,1024)
	$(call makeimage,mantis-original.png,iTunesArtwork@2x,1024,1024)
	$(call makeimage,mantis-original.png,mantisicon57x57,57,57)
	$(call makeimage,mantis-original.png,Icon,57,57)
	$(call makeimage,mantis-original.png,mantisicon114x114,114,114)
	$(call makeimage,mantis-original.png,Icon@2x,114,114)
	$(call makeimage,mantis-original.png,mantisicon72x72,72,72)
	$(call makeimage,mantis-original.png,Icon-72,72,72)
	$(call makeimage,mantis-original.png,mantisicon144x144,144,144)
	$(call makeimage,mantis-original.png,Icon-72@2x,144,144)
	$(call makeimage,mantis-original.png,mantisicon29x29,29,29)
	$(call makeimage,mantis-original.png,Icon-Small,29,29)
	$(call makeimage,mantis-original.png,mantisicon58x58,58,58)
	$(call makeimage,mantis-original.png,Icon-Small@2x,58,58)
	$(call makeimage,mantis-original.png,mantisicon50x50,50,50)
	$(call makeimage,mantis-original.png,Icon-Small-50,50,50)
	$(call makeimage,mantis-original.png,mantisicon100x100,100,100)
	$(call makeimage,mantis-original.png,Icon-Small-50@2x,100,100)
	$(call makeimage,mantis-original.png,mantisicon60x60,60,60)
	$(call makeimage,mantis-original.png,mantisicon120x120,120,120)
	$(call makeimage,mantis-original.png,mantisicon40x40,40,40)
	$(call makeimage,mantis-original.png,mantisicon80x80,80,80)

iPadIcons:
	$(call makeimage,mantis-original.png,mantisicon512x512,512,512)
	$(call makeimage,mantis-original.png,iTunesArtwork,512,512)
	$(call makeimage,mantis-original.png,mantisicon1024x1024,1024,1024)
	$(call makeimage,mantis-original.png,iTunesArtwork@2x,1024,1024)
	$(call makeimage,mantis-original.png,mantisicon72x72,72,72)
	$(call makeimage,mantis-original.png,Icon-72,72,72)
	$(call makeimage,mantis-original.png,mantisicon144x144,144,144)
	$(call makeimage,mantis-original.png,Icon-72@2x,144,144)
	$(call makeimage,mantis-original.png,mantisicon29x29,29,29)
	$(call makeimage,mantis-original.png,Icon-Small,29,29)
	$(call makeimage,mantis-original.png,mantisicon58x58,58,58)
	$(call makeimage,mantis-original.png,Icon-Small@2x,58,58)
	$(call makeimage,mantis-original.png,mantisicon50x50,50,50)
	$(call makeimage,mantis-original.png,Icon-Small-50,50,50)
	$(call makeimage,mantis-original.png,mantisicon100x100,100,100)
	$(call makeimage,mantis-original.png,Icon-Small-50@2x,100,100)
	$(call makeimage,mantis-original.png,mantisicon76x76,76,76)
	$(call makeimage,mantis-original.png,mantisicon152x152,152,152)
	$(call makeimage,mantis-original.png,mantisicon40x40,40,40)
	$(call makeimage,mantis-original.png,mantisicon80x80,80,80)

universalIcons:
	@$(call makeimage,mantis-original.png,mantisicon512x512,512,512)
	@$(call makeimage,mantis-original.png,iTunesArtwork,512,512)
	@$(call makeimage,mantis-original.png,mantisicon1024x1024,1024,1024)
	@$(call makeimage,mantis-original.png,iTunesArtwork@2x,1024,1024)
	@$(call makeimage,mantis-original.png,mantisicon57x57,57,57)
	@$(call makeimage,mantis-original.png,Icon,57,57)
	@$(call makeimage,mantis-original.png,mantisicon114x114,114,114)
	@$(call makeimage,mantis-original.png,Icon@2x,114,114)
	@$(call makeimage,mantis-original.png,mantisicon72x72,72,72)
	@$(call makeimage,mantis-original.png,Icon-72,72,72)
	@$(call makeimage,mantis-original.png,mantisicon144x144,144,144)
	@$(call makeimage,mantis-original.png,Icon-72@2x,144,144)
	@$(call makeimage,mantis-original.png,mantisicon29x29,29,29)
	@$(call makeimage,mantis-original.png,Icon-Small,29,29)
	@$(call makeimage,mantis-original.png,mantisicon58x58,58,58)
	@$(call makeimage,mantis-original.png,Icon-Small@2x,58,58)
	@$(call makeimage,mantis-original.png,mantisicon50x50,50,50)
	@$(call makeimage,mantis-original.png,Icon-Small-50,50,50)
	@$(call makeimage,mantis-original.png,mantisicon100x100,100,100)
	@$(call makeimage,mantis-original.png,Icon-Small-50@2x,100,100)
	@$(call makeimage,mantis-original.png,mantisicon60x60,60,60)
	@$(call makeimage,mantis-original.png,mantisicon120x120,120,120)
	@$(call makeimage,mantis-original.png,mantisicon76x76,76,76)
	@$(call makeimage,mantis-original.png,mantisicon152x152,152,152)
	@$(call makeimage,mantis-original.png,mantisicon40x40,40,40)
	@$(call makeimage,mantis-original.png,mantisicon80x80,80,80)

iPhoneLaunchScreens:
	@$(call makelaunch,launchscreen-origpo.png,Default,320,480)
	@$(call makelaunch,launchscreen-origpo.png,Default~iphone,320,480)
	@$(call makelaunch,launchscreen-origpo.png,Default@2x,640,960)
	@$(call makelaunch,launchscreen-origpo.png,Default-568h@2x,640,1136)

iPadLaunchScreens:
	@$(call makelaunch,launchscreen-origpo.png,Default-Portrait~ipad,768,1024)
	@$(call makelaunch,launchscreen-origpo.png,Default-Portrait@2x~ipad,1536,2048)
	@$(call makelaunch,launchscreen-origls.png,Default-Landscape~ipad,1024,768)
	@$(call makelaunch,launchscreen-origls.png,Default-Landscape@2x~ipad,2048,1536)

universalLaunchScreens: iPhoneLaunchScreens iPadLaunchScreens


cp:
	cp ../*.*shader BoomBoomPow.app-debug/
	cp ../*.tga  BoomBoomPow.app-debug/
	cp ../*.level ../*.actor  BoomBoomPow.app-debug/

