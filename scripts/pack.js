const fse = require('fs-extra');
const path = require('path');
const glob = require('glob');
const rimraf = require('rimraf');

const dist_dir = "dist";
const dist_dir_path = path.join(__dirname, "..", dist_dir);
const prepack_path  = path.join(dist_dir_path, "game-overlay");
const npm_dir_path  = path.join(__dirname, "..", 'npm');
const module_name   = 'streamlabs_overlay.node';
const module_search_path = path.join(__dirname, '..', 'build*', 'Release', module_name);
var node_module_path = '';
var pack_version = process.env.MODULE_VERSION;
// Make dist dir and move files to it 
try {
  fse.ensureDir(dist_dir_path);
  rimraf.sync(dist_dir_path + "/*");
  fse.ensureDir(prepack_path);

  fse.copySync(npm_dir_path, prepack_path);
  console.log('Search for module with line : ' + module_search_path);

  var modules_files = glob.sync(module_search_path, {});
  console.log('Found module : ' + modules_files);
  node_module_path = modules_files[0];
  fse.copySync(node_module_path, path.join(prepack_path, module_name));

} catch (err) {
  console.error(err)
}

// Get version and save to relase package.json 
try {
  console.log('Releasing version : ' + pack_version);

  let release_package_rawdata = fse.readFileSync(path.join(prepack_path, 'package.json'));  
  let release_package_info = JSON.parse(release_package_rawdata);  
  release_package_info.version = pack_version;
  let release_data = JSON.stringify(release_package_info);  
  fse.writeFileSync(path.join(prepack_path, 'package.json'), release_data);  
} catch (err) {
  console.error(err)
}

const release_package = 'game-overlay-v'+pack_version+'.tar.gz';
const package_path = path.join(__dirname, '..', release_package);


console.log('Dist path : ' + dist_dir_path);
console.log('Release package : ' +package_path );
// Make tar.gz from dist dir 
var pack = require('tar')

var pack_files = glob.sync(prepack_path+'\\*', {}).map( function(cur_obj) {
  return cur_obj.substr(dist_dir_path.length+1);
} );
console.log('Files to be packed : ' + pack_files);
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
