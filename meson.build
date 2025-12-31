# meson.build
# toq 2025-12-31
# Project definition (App ID)
project('free.toq.finden', 'c', meson_version: '>= 0.59', 
  default_options: [
    'warning_level=3',
    'werror=false',
    'c_std=c11',
    'buildtype=release',
    'prefix=/usr/'
  ]
)

# Base structure
src_dir  = 'src'
data_dir = 'data'
icons_dir = join_paths(data_dir, 'icons') # Icons
po_dir   = join_paths(src_dir, 'po')      # PO files
source_root = meson.global_source_root()

# GResource XML definition
gresource_xml = join_paths(src_dir, 'free.toq.finden.gresource.xml')

# Dependencies
glib_dep    = dependency('glib-2.0', required : true)
gtk_dep     = dependency('gtk4', required : true)
adwaita_dep = dependency('libadwaita-1', version: '>= 1.8.alpha', required : true)
gnome = import('gnome')

# Remove "unused" warnings
add_project_arguments(
 '-w',
 language : 'c'
)

# Generate ressources file
gresource_c = custom_target(
  'gresource',
  output : 'free.toq.finden-gresource.c',
  command : ['glib-compile-resources', join_paths(source_root, 'src', 'resources.xml'),
                      '--sourcedir=' + join_paths(source_root, 'data', 'icons'),
                      '--generate-source',
                      '--target=' + 'free.toq.finden-gresource.c']
)

# Executable
executable('toq-finden',
  sources : [
    join_paths(src_dir, 'main.c'),
    join_paths(src_dir, 'config.c'),
    gresource_c
  ],
  dependencies : [glib_dep, gtk_dep, adwaita_dep],
  install : true,
  install_dir : join_paths(get_option('prefix'), 'bin')
)


# Sub - Miniterm Path
# subdir('src/miniterm')

# Install Icon (App ID)
install_data(join_paths(icons_dir, 'free.toq.finden.svg'),
 install_dir : join_paths(get_option('prefix'), 'share', 'icons', 'hicolor', 'scalable', 'apps'))
# install_data(join_paths(icons_dir, 'free.toq.miniterm.svg'),
# install_dir : join_paths(get_option('prefix'), 'share', 'icons', 'hicolor', 'scalable', 'apps'))

# Install .desktop file (App ID)
install_data(join_paths(data_dir, 'free.toq.finden.desktop.download'),
 install_dir: join_paths(get_option('prefix'), 'share', 'applications'),
 rename     : 'free.toq.finden.desktop')
# install_data(join_paths(data_dir, 'free.toq.miniterm.desktop.download'),
# install_dir: join_paths(get_option('prefix'), 'share', 'applications'),
# rename     : 'free.toq.miniterm.desktop')

# Localization files
# EN_US 
po_file_en_US = join_paths(po_dir, 'toq-finden_en_US.po')
mo_file = 'toq-finden.mo'
run_command('msgfmt', po_file_en_US, '--output-file=' + join_paths('en_US.mo'), check : true)
install_data(join_paths( 'en_US.mo'),
  install_dir: join_paths(get_option('prefix'), 'share', 'locale', 'en_US', 'LC_MESSAGES'),
  rename: mo_file
)
# EN_GB
po_file_en_GB = join_paths(po_dir, 'toq-finden_en_GB.po')
mo_file = 'toq-finden.mo'
run_command('msgfmt', po_file_en_GB, '--output-file=' + join_paths('en_GB.mo'), check : true)
install_data(join_paths( 'en_GB.mo'),
  install_dir: join_paths(get_option('prefix'), 'share', 'locale', 'en_GB', 'LC_MESSAGES'),
  rename: mo_file
)
# PL
po_file_pl_PL = join_paths(po_dir, 'toq-finden_pl_PL.po')
run_command('msgfmt', po_file_pl_PL, '--output-file=' + join_paths('pl_PL.mo'), check : true)
install_data(join_paths( 'pl_PL.mo'),
  install_dir: join_paths(get_option('prefix'), 'share', 'locale', 'pl_PL', 'LC_MESSAGES'),
  rename: mo_file
)

# Update Cache
gnome.post_install( gtk_update_icon_cache: true, update_desktop_database: true,)

