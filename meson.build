# meson.build
# toq 2025.11.15
# Project definition (App ID)
project('free.toq.finden', 'c', version: '1.0.0', default_options: 
 [
 'warning_level=3',
 'c_std=c11',
 'buildtype=release',
 'prefix=/usr/'
 ]
)

# Base structure
src_dir  = 'src'
data_dir = 'data'
icons_dir = join_paths(data_dir, 'icons') # Icons
po_dir   = join_paths(src_dir, 'po')     # PO files

# GResource XML definition
gresource_xml = join_paths(data_dir, 'free.toq.finden.gresource.xml')

# Dependencies
glib_dep    = dependency('glib-2.0', required : true)
gtk_dep     = dependency('gtk4', required : true)
adwaita_dep = dependency('libadwaita-1', required : true)
gnome = import('gnome')

# Remove "unused" warnings, without code changes.
add_project_arguments(
 '-Wno-unused-variable',
 '-Wno-unused-parameter',
 '-Wno-missing-parameter-name',
 '-Wno-missing-field-initializers',
 '-Wno-overlength-strings',
 language : 'c'
)

# Gen. ressources
#resource = gnome.gresource(gresource_xml, 
#  dependencies: [glib_dep],
#  install: true
#)

# Build and install (App ID)
executable('free.toq.finden',
 sources     : [join_paths(src_dir, 'main.c'), 'src/free.toq.finden.gresource.c'],
 dependencies: [glib_dep, gtk_dep, adwaita_dep],
 install     : true, 
 install_dir : join_paths(get_option('prefix'), 'bin')
)

# Sub - Miniterm Path
subdir('src/miniterm')

# Install Icon (App ID)
install_data(join_paths(icons_dir, 'free.toq.finden.svg'),
 install_dir : join_paths(get_option('prefix'), 'share', 'icons', 'hicolor', 'scalable', 'apps'))
install_data(join_paths(icons_dir, 'free.toq.miniterm.svg'),
 install_dir : join_paths(get_option('prefix'), 'share', 'icons', 'hicolor', 'scalable', 'apps'))

# Install .desktop file (App ID)
install_data(join_paths(data_dir, 'free.toq.finden.desktop.download'),
 install_dir: join_paths(get_option('prefix'), 'share', 'applications'),
 rename     : 'free.toq.finden.desktop')

# Localization files
# EN 
po_file_en = join_paths(po_dir, 'toq-finden_en_US.po')
mo_file = 'toq-finden.mo'
run_command('msgfmt', po_file_en, '--output-file=' + 'toq-finden_en_US.mo', check : true)

install_data('toq-finden_en_US.mo',
 install_dir: join_paths(get_option('prefix'), 'share', 'locale', 'en_US', 'LC_MESSAGES'),
 rename     : mo_file)

# PL
po_file_pl = join_paths(po_dir, 'toq-finden_pl_PL.po')
run_command('msgfmt', po_file_pl, '--output-file=' + 'toq-finden_pl_PL.mo', check : true)

install_data('toq-finden_pl_PL.mo',
 install_dir: join_paths(get_option('prefix'), 'share', 'locale', 'pl_PL', 'LC_MESSAGES'),
 rename     : mo_file)

