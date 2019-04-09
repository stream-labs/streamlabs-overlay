const fse = require('fs-extra')

try {
    fse.ensureDir('dist')
    
    fse.copySync('npm/*', 'dist/')
    
  } catch (err) {
    console.error(err)
  }

var write = require('fs').createWriteStream

var pack = require('tar-pack').pack

pack(process.cwd())
  .pipe(write(__dirname + '/package.tar.gz'))
  .on('error', function (err) {
    console.error(err.stack)
  })
  .on('close', function () {
    console.log('done')
  })