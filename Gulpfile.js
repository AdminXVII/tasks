var gulp = require('gulp');
var sass = require('gulp-sass');
var svgSymbols = require('gulp-svg-symbols');
var htmlmin = require('gulp-htmlmin');
var svgo = require('gulp-svgo');
var gls = require('gulp-live-server');
var purge = require('gulp-css-purge');
var uncache = require('gulp-uncache');

gulp.task('imgs', function () {
  return gulp.src('src/img/*.svg')
    .pipe(svgSymbols())
    .pipe(svgo({
      plugins: [{
        cleanupIDs: false
      }]
    }))
    .pipe(gulp.dest('dist'));
});

gulp.task('js', function () {
  return gulp.src('./src/*.js')
    .pipe(gulp.dest('./dist/'));
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

gulp.task('dev', ['html','sass','imgs', 'js', 'static'], function(){

  var server = gls.static('dist');
  server.start();

  gulp.watch(['./src/index.html','./src/content.md', './src/profile.md'], ['html']);
  gulp.watch('./src/img/*.svg', ['imgs']);
  gulp.watch('./src/sass/*.scss', ['sass']);
  gulp.watch('./src/*.js', ['js']);
  gulp.watch('./src/static/*', ['static']);
});

gulp.task('default', ['imgs','sass','html', 'js']);
