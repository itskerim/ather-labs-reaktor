#!/usr/bin/env node
/**
 * Build AETHER 3.0 User Manual as PDF (explicit output path).
 * Usage: node build_pdf.js
 * Requires: md-to-pdf (npx will install if needed).
 */

const path = require('path');
const { execSync } = require('child_process');

const dir = __dirname;
const mdPath = path.join(dir, 'AETHER_3.0_User_Manual.md');
const pdfPath = path.join(dir, 'AETHER_3.0_User_Manual.pdf');
const cssPath = path.join(dir, 'aether-manual.css');

console.log('Building AETHER_3.0_User_Manual.pdf (AETHER style) ...');
console.log('  Input:  ', mdPath);
console.log('  Output: ', pdfPath);

const fs = require('fs');
try {
  execSync(
    `npx --yes md-to-pdf "${mdPath}" --stylesheet "${cssPath}"`,
    {
      cwd: dir,
      stdio: 'inherit',
    }
  );
  // md-to-pdf writes to same path as input but with .pdf extension
  const actualPdf = mdPath.replace(/\.md$/i, '.pdf');
  if (fs.existsSync(actualPdf)) {
    console.log('Done. PDF written to:', actualPdf);
  } else {
    console.error('Error: PDF was not created. Expected:', actualPdf);
    process.exit(1);
  }
} catch (err) {
  console.error('Build failed:', err.message);
  process.exit(1);
}
