
# make this config persistent, to share infos at runtime -> makes in header reference possible
configure_file ( output : meson.project_name() + '_config.h'
               , configuration : conf
               , install : true
               , install_dir : 'include/' + meson.project_name())
project_headers = [
	 'ConcurrentCollections.hpp'
	,'LocaleContext.hpp'
	,'ImageUtils.hpp'
	,'ImageView.hpp'
	,'BinModel.hpp'
	,'BinView.hpp'
	,'ImageOptions.hpp'
	,'ImageFileChooser.hpp'
	,'ImageOptionDialog.hpp'
	,'ImageArea.hpp'
	,'ExifReader.hpp'
	,'ImageList.hpp'
	,'DisplayImage.hpp'
	,'ApplicationSupport.hpp'
	,'Mode.hpp'
	,'JsonHelper.hpp'
	,'JsonObj.hpp'
	,'Log.hpp'
	,'LogImpl.hpp'
	,'LogView.hpp'
	,'LogViewSysd.hpp'
	,'LogViewFile.hpp'
	,'LogViewSyslog.hpp'
	,'DateUtils.hpp'
	,'psc_format.hpp'
	,'psc_i18n.hpp'
	,'AbstractTableManager.hpp'
	,'KeyfileTableManager.hpp'
	,'TableProperties.hpp'
	,'StringUtils.hpp'
	,'psc_Files.hpp'
	,'ThreadWorker.hpp'
	,'TreeNodeModel.hpp'
	,'Plot.hpp'
	,'KeyConfig.hpp' ]

if libharu_deps.found()
    project_headers += [
	 'PdfExport.hpp'
	,'PdfFont.hpp'
	,'PdfImage.hpp'
	,'PdfPage.hpp' ]
endif

# Make this library usable from the system's
# package manager.
install_headers(project_headers, subdir : meson.project_name())
