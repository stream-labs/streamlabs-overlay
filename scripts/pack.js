const fse = require('fs-extra');
const path = require('path');
const glob = require('glob');
const rimraf = require('rimraf');

const dist_dir = "dist";
const dist_dir_path = path.join(__dirname, "..", dist_dir);
const release_package = "streamlabs_game_overlay.tar.gz";
const package_path = path.join(__dirname, '..', release_package);
const npm_dir_path = path.join(__dirname, "..", 'npm');
const module_name = 'streamlabs_overlay.node';
const module_search_path = path.join(__dirname, '..', 'build*', 'Release', '**', module_name);
var node_module_path = '';
// Make dist dir and move files to it 
try {
  fse.ensureDir(dist_dir_path);
  rimraf.sync(dist_dir_path + "/*");

  fse.copySync(npm_dir_path, dist_dir_path);
  console.log('search for module with ' + module_search_path);

  var modules_files = glob.sync(module_search_path, {});
  console.log('Found module ' + modules_files);
  node_module_path = modules_files[0];
  fse.copySync(node_module_path, path.join(dist_dir_path, module_name));

} catch (err) {
  console.error(err)
}

console.log('dist path ' + dist_dir_path);
console.log('npm path ' + npm_dir_path);
console.log('node module path ' + node_module_path);

// Make tar.gz from dist dir 
var pack = require('tar')

var pack_files = glob.sync(dist_dir_path+'\\*', {}).map( function(cur_obj) {
  return cur_obj.substr(dist_dir_path.length+1);
} );
console.log('files to pack ' + pack_files);
pack.c(
  {
    cwd: dist_dir_path,
    sync: true,
    preservePaths:false,
    gzip: true,
    file: package_path
  },
  pack_files
);
