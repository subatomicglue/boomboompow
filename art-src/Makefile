
all: art

makeimage = rm -f $(2).$(5); cp $(1) $(2).$(5); sips -z $(4) $(3) $(2).$(5) > /dev/null

art:
	@$(call makeimage,pro-boomboompow-bg.jpg,../pro/boomboompow-bg-1024,1024,1024,jpg)
	@$(call makeimage,pro-boomboompow-splash-ls.png,../pro/boomboompow-splash-ls-1024,1024,768,png)
	@$(call makeimage,pro-boomboompow-splash-po.png,../pro/boomboompow-splash-po-1024,768,1024,png)
	@$(call makeimage,kid-boomboompow-bg.jpg,../kid/boomboompow-bg-1024,1024,1024,jpg)
	@$(call makeimage,kid-boomboompow-splash-ls.png,../kid/boomboompow-splash-ls-1024,1024,768,png)
	@$(call makeimage,kid-boomboompow-splash-po.png,../kid/boomboompow-splash-po-1024,768,1024,png)
	@$(call makeimage,kid-boomboompow-bg.jpg,../kid2/boomboompow-bg-1024,1024,1024,jpg)
	@$(call makeimage,kid-boomboompow-splash-ls.png,../kid2/boomboompow-splash-ls-1024,1024,768,png)
	@$(call makeimage,kid-boomboompow-splash-po.png,../kid2/boomboompow-splash-po-1024,768,1024,png)

kid: art
	cp ../kid/boomboompow-splash-ls-1024.png ../iOS/launchscreen-origls.png
	cp ../kid/boomboompow-splash-po-1024.png ../iOS/launchscreen-origpo.png

kid2: art
	cp ../kid2/boomboompow-splash-ls-1024.png ../iOS/launchscreen-origls.png
	cp ../kid2/boomboompow-splash-po-1024.png ../iOS/launchscreen-origpo.png

pro: art
	cp ../pro/boomboompow-splash-ls-1024.png ../iOS/launchscreen-origls.png
	cp ../pro/boomboompow-splash-po-1024.png ../iOS/launchscreen-origpo.png

