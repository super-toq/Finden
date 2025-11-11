# miniterm, toq 2025
# src/miniterm/meson.build
miniterm_src = files('miniterm.c')

gtk_dep     = dependency('gtk4', required : true)
adwaita_dep = dependency('libadwaita-1', required : true)
glib_dep    = dependency('glib-2.0', required : true)

executable('free.toq.miniterm',
  sources     : miniterm_src,
  dependencies: [glib_dep, gtk_dep, adwaita_dep],
  install     : true,
  install_dir : join_paths(get_option('prefix'), 'bin')
)
