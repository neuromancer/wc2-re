Trusted anchors: 945; bounded gaps: 857; equal-count gaps: 718; unequal gaps: 139; inversions skipped: 43; oversized gaps: 30
Order is a candidate signal only; validate proposed destinations with binary-comp before updating the map.

src/gr.c: 00442460 Streamer_stop -> 00428A6D .. 004424D0 SetStreamerTrigger -> 00428AEB
  gap counts WC1=1 WC2=1 (equal)
   1. 004424B0 ClearStreamerTrigger -> 00428AAB SetStreamerIntensity [candidate-target-conflict; target-owned-by=00442520]

src/hudmsg.c: 00429FC0 RenderSpaceViewFrame -> 0046903F .. 0042A0E0 GetShipDistanceToNavPoint -> 00469143
  gap counts WC1=1 WC2=1 (equal)
   1. 0042A0C0 RefreshCockpitStatus -> 004690FF Draw_3Space_Frame [candidate-target-conflict; target-owned-by=00429DD0]

src/mathfp.c: 00434FF0 GetFontCharWidth -> 00461DBD .. 00435090 IsPointInRect -> 00461E97
  gap counts WC1=2 WC2=2 (equal)
   1. 00435010 ReleaseVideoResourcesHook -> 00461DE0 ApplySpacePaletteModeHook [candidate-unmapped]

src/music.c: 0042EEE0 EnableMusicForScene -> 004534FC .. 0042EF10 FlushSoundEffectsAndLog -> 0045357E
  gap counts WC1=1 WC2=1 (equal)
   1. 0042EF00 SoundFxTick -> 0045351A LogUnknownSoundEffect [candidate-unmapped]

src/ship.c: 0041FB40 analyze_kill -> 00413F58 .. 00420190 send_at_point -> 00414BE9
  gap counts WC1=5 WC2=5 (equal)
   1. 0041FBC0 ShipExplosion -> 00414025 explosion_shock_wave [source-and-target-conflict; current=004142C8; target-owned-by=0041FEE0; current-score=81.13%]
   2. 0041FCD0 Explosion -> 004142C8 ShipExplosion [source-and-target-conflict; current=0041444C; target-owned-by=0041FBC0; current-score=54.94%]
   3. 0041FEB0 the_creator -> 0041444C Explosion [candidate-target-conflict; target-owned-by=0041FCD0]
   4. 0041FEE0 explosion_shock_wave -> 00414835 explode [candidate-remap; current=00414025; current-score=69.73%]
   5. 00420040 explode -> 004148F5 ResolveObjectDestruction [candidate-unmapped]

src/strdos.c: 00435470 DosStrcpy -> 004624A7 .. 004354D0 DosStrlen -> 0046250B
  gap counts WC1=1 WC2=1 (equal)
   1. 004354A0 CopyFarString -> 004624C7 CopyStringAndReturnEnd [candidate-target-conflict; target-owned-by=00461AB0]

src/winmain.c: 00401000 SaveGamePalette -> 00417550 .. 004010C0 make_shard -> 004176D2
  gap counts WC1=2 WC2=2 (equal)
   1. 00401020 RestoreGamePalette -> 004175AD FUN_004175AD [candidate-unmapped]
   2. 00401040 easy2see -> 00417610 FUN_00417610 [candidate-unmapped]

Displayed candidate/context rows: 12
Wrote complete order audit TSV: /Users/g/Reversing/WC/wc2-re/reports/wc2-order-audit.tsv
