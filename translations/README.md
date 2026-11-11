# External translations

Choose **Language → Español** or **Language → Polski** in the launcher, or
start with `--language=es` or `--language=pl`. Use `--language=en` for
English. The launcher remembers the selection; command-line language options
override its saved setting. Changing the language also updates the launcher
immediately.

Spanish covers 3,337 base-game cutscene dialogue entries, 158 marked game UI
strings, radio text, mission and navigation labels, menu artwork, opening
credits and scrolling credits, using DOS or Kilrathi Saga data. The
upstream translation's reordered COMMUNIC.T29 packets and Special Operations
resources are excluded, so SO1 and SO2 remain in English with it.

Polish covers the base game and both Special Operations: 5,652 cutscene
dialogue entries, radio text, mission objectives, menu labels and button
artwork, and 156 game UI strings, using DOS or Kilrathi Saga data. The
intro's title cards, which the upstream patch leaves in English, are
redrawn in Polish from the original title letters. The scrolling credits'
sprite alphabet has no Polish letters, so they appear there as base letters.
Voices remain unchanged in both languages.

The game reads `translations/es.dat` or `translations/pl.dat` beside its
executable, or under `Contents/Resources` in the macOS app bundle. Editable
UTF-8 PO files are compiled into WC2's single-byte text and accent glyphs,
following wc1-re's WCT2 catalog approach. English fallback comes from the
user's original game data and the recovered source. Game files on disk are
never rewritten. Text, font glyphs and selected image frames change in
memory; scripts, timing, resource indices, hotspot commands and existing
community fixes are preserved. Artwork changes require the expected packet
checksum and leave unrelated frames and palettes intact. Opening credits
apply after the community director-credit correction, preserving the
corrected name; caption placement is included in the images, leaving the
scripts unchanged. An expansion is translated only by a catalog that
includes its dialogue, so the Spanish catalog does not mix its base-game
text into SO1 or SO2.

Edit `es/` or `pl/` `cutscenes.po`, `resources.po` or `ui.po`, preserving
`msgctxt`, `msgid`, placeholders such as `{callsign}` and `{last_name}`,
formatting directives and shortcut letters. IDs hash the original bytes
instead of copying English dialogue into the catalog. Empty or fuzzy entries
fall back to English. Compilation rejects changed placeholders, oversized
dialogue or resource fields, and invalid glyph definitions. A translation may
omit trailing format directives, such as an English plural's `%c`, because
the game ignores surplus arguments; `msgfmt -c` still reports those entries.
Each language's `glyphs.json` selects its code page: CP1252 for Spanish and
CP1250 for Polish. WC2's six control bytes remain reserved in cutscenes.

```sh
make modern-translations          # rebuild the external game catalogs without relinking
make modern modern-gui            # also build the executables
make update-translations         # refresh marked game UI strings; requires GNU gettext
make update-launcher-translations # refresh Slint strings; also requires slint-tr-extractor
```

Mark new game UI strings with `_()` or `C_(context, text)`. Static initializers
can use `N_()`/`NC_()`, with translation performed at display time. These macros
preserve the original expressions in the reference build. Mark launcher strings
with Slint's `@tr()` and edit each language's `LC_MESSAGES/launcher.po`; these
are bundled into the launcher when it is rebuilt. Keep keyboard shortcut letters
unchanged even when the surrounding text is translated.

The imported Spanish text, artwork and glyphs are by **L@Zar0** and **darkpadawan**, from
version 1.1 (2021). The [upstream notice](upstream/Leeme.txt) declares GPL v3.0
with an additional no-charge term. [provenance.json](provenance.json) records
the download, hashes, one restored callsign placeholder and three alternative
IDs for Saga's line-wrap changes. Game UI and launcher translations reuse the
matching wc1-re catalog entries, with additions for WC2; see their PO headers.

The imported Polish text, artwork and glyphs are by **paxiu1984**, from patch
version 1.4 (2026), distributed as `WC2_Spolszczenie_PL_v1.1_latka.zip`,
which builds on L@Zar0 and darkpadawan's Spanish version.
paxiu1984 permitted their use here under GPL v3.0 with an additional
no-charge term, as for the Spanish translation; the
[upstream notice](upstream/CZYTAJ_MNIE.txt) accompanies the catalog.
[pl/provenance.json](pl/provenance.json) records the archive hash, the
English inputs and the corrections: letters move from the punctuation slots
the patch repurposes to CP1250, four punctuation marks it encodes with those
slots are kept as punctuation, and shortcut keys stay as in the original
(the patch moved Yes to T). Game UI strings come from the translators'
spreadsheet. Twelve UI strings that the patch does not cover, the launcher
and the intro title cards are local additions, marked as such.

The editable catalogs contain the imported text and artwork changes; builds
need neither the upstream archives nor an import tool. The imports exclude
executables, voices, gameplay changes and the Polish voice-subtitle file.
One Python script, `bin/compileTranslations.py`, compiles game catalogs and
refreshes marked UI strings. Release archives and both installers include
the compiled catalogs and both upstream notices.
