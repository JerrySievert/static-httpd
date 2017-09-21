#!/usr/bin/env node

const fs = require('fs');
const types = require('./db.json');

const keys = Object.keys(types);

let cfile = fs.openSync(__dirname + '/../src/mime_types.c', 'w');
let hfile = fs.openSync(__dirname + '/../include/mime_types.h', 'w');

let extensions = [ ];
let mime = [ ];

keys.forEach((type) => {
  if (types[type].extensions) {
    types[type].extensions.forEach((extension) => {
      extensions.push('  "' + extension + '"');
      mime.push('  "' + type + '"');
    });
  }
});

fs.writeSync(hfile, '#pragma once\n\n', 'utf8');
fs.writeSync(hfile, '#define MIME_LENGTH ' + extensions.length + '\n\n', 'utf8');

fs.writeSync(cfile, '#include "mime_types.h"\n\n', 'utf8');
fs.writeSync(cfile, 'const char *mime_extensions[MIME_LENGTH] = {\n');

fs.writeSync(cfile, extensions.join(',\n'), 'utf8');
fs.writeSync(cfile, '\n};\n\n', 'utf8');

fs.writeSync(cfile, 'const char *mime_types[MIME_LENGTH] = {\n');

fs.writeSync(cfile, mime.join(',\n'), 'utf8');
fs.writeSync(cfile, '\n};\n', 'utf8');

fs.close(cfile);
fs.close(hfile);
