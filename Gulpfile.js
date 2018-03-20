var gulp = require('gulp');
var sass = require('gulp-sass');
var svgSymbols = require('gulp-svg-symbols');
var htmlmin = require('gulp-htmlmin');
var svgo = require('gulp-svgo');
var gls = require('gulp-live-server');
var purge = require('gulp-css-purge');
var uncache = require('gulp-uncache');
const workboxBuild = require('workbox-build');

gulp.task('imgs', function () {
  return gulp.src('src/img/*.svg')
    .pipe(svgo())
    .pipe(gulp.dest('dist/img/'));
});

gulp.task('sw', () => {
  return workboxBuild.generateSW({
    //swSrc: 'src/sw.js',
    swDest: 'dist/sw.js',
    globDirectory: 'dist',
    globPatterns: ['**\/*.{js,css,html,png,svg,json}'],
    runtimeCaching: [
      {
        urlPattern: /^https:\/\/code\.getmdl\.io/,
        handler: 'staleWhileRevalidate'
      },
      {
        urlPattern: /^https:\/\/fonts\.googleapis\.com/,
        handler: 'staleWhileRevalidate'
      },
      {
        urlPattern: /^https:\/\/fonts\.gstatic\.com/,
        handler: 'staleWhileRevalidate'
      }
    ]
  });
});

gulp.task('js', function () {
  return gulp.src('./src/js/*.js')
    .pipe(gulp.dest('./dist/js/'));
});

gulp.task('static', function () {
  return gulp.src('./src/static/*')
    .pipe(gulp.dest('./dist/'));
});

gulp.task('sass', function () {
  return gulp.src('./src/sass/*.scss')
    .pipe(sass({ style: 'compressed' }).on('error', sass.logError))
    .pipe(purge({
        shorten : false,
        shorten_zero : false,
        shorten_hexcolor_extended_names: true,
        shorten_font: true,
        shorten_background: true,
        shorten_background_min: 2,
        shorten_margin: true,
        shorten_padding: true,
        shorten_list_style: true,
    }))
    .pipe(gulp.dest('./dist/css'));
});

gulp.task('html', function() {
  return gulp.src('src/*.html')
    .pipe(uncache())
    .pipe(htmlmin({collapseWhitespace: true}))
    .pipe(gulp.dest('dist'));
});

gulp.task('default', ['imgs','sass','html', 'js', 'static', 'sw']);

gulp.task('dev', ['default'], function(){

  var server = gls.static('dist');
  server.start();

  gulp.watch('./src/index.html', ['html','sw']);
  gulp.watch('./src/img/*.svg', ['imgs','sw']);
  gulp.watch('./src/sass/*.scss', ['sass','sw']);
  gulp.watch('./src/js/*.js', ['js','sw']);
  gulp.watch('./src/static/*', ['static','sw']);
});