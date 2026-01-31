# AETHER 3.0 Documentation

This folder contains the **AETHER 3.0 User Manual** and instructions for turning it into a finished PDF or web doc.

## Files

- **AETHER_3.0_User_Manual.md** — Full user manual (20+ pages when exported). Covers what AETHER is, why it was made, installation, interface, every control, signal flow, presets, tips, specs, roadmap, troubleshooting, and appendices.

## Adding Screenshots

The manual uses placeholders like:

```text
[SCREENSHOT: Full AETHER 3.0 interface — main window with all sections visible.]
```

To add real screenshots:

1. Capture screenshots of the AETHER plugin (e.g. full UI, sections, knobs, presets).
2. Save them in this folder (e.g. `docs/images/`) with clear names (e.g. `interface-full.png`, `distortion-section.png`).
3. In the manual, replace each placeholder with Markdown image syntax:

   ```markdown
   ![Full AETHER 3.0 interface](images/interface-full.png)
   ```

4. Adjust the path if you use a different folder structure.

## Exporting to PDF (20+ pages)

**Option 1: md-to-pdf (recommended, no LaTeX required)**

From the `docs` folder:

```bash
npx --yes md-to-pdf AETHER_3.0_User_Manual.md
```

Or run the script:

```bash
./build_pdf.sh
```

Output: `AETHER_3.0_User_Manual.pdf` in the same folder.

**Option 2: Pandoc** (requires Pandoc + LaTeX)

```bash
pandoc AETHER_3.0_User_Manual.md -o AETHER_3.0_User_Manual.pdf --pdf-engine=xelatex -V geometry:margin=1in
```

**Option 3: Typora or similar**

Open the `.md` file in Typora (or another Markdown editor with PDF export), add your images, then use **File → Export → PDF**.

**Option 4: VS Code / Cursor**

Use a Markdown PDF extension: open the manual, add images, then export to PDF from the editor.

**Option 5: Static site (e.g. MkDocs)**

Place the manual in a MkDocs (or similar) project and build the site; then print to PDF from the browser if you want a web-first version.

For a true 20+ page count, use normal body font size (11–12 pt) and 1 in margins; the manual text and structure are written to fill that length once screenshots are in place.
