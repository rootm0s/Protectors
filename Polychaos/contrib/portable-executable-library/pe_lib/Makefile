OBJS = entropy.o file_version_info.o message_table.o pe_base.o pe_bound_import.o pe_checksum.o pe_debug.o pe_directory.o pe_dotnet.o pe_exception_directory.o pe_exports.o pe_imports.o pe_load_config.o pe_properties.o pe_properties_generic.o pe_relocations.o pe_factory.o pe_resources.o pe_resource_manager.o pe_resource_viewer.o pe_rich_data.o pe_section.o pe_tls.o utils.o version_info_editor.o version_info_viewer.o pe_exception.o resource_message_list_reader.o resource_string_table_reader.o resource_version_info_reader.o resource_version_info_writer.o resource_cursor_icon_reader.o resource_cursor_icon_writer.o resource_bitmap_writer.o resource_bitmap_reader.o resource_data_info.o pe_rebuilder.o
LIBNAME = pebliss
LIBPATH = ../lib
CXXFLAGS = -O2 -Wall -fPIC -DPIC -I.

ifdef PE_DEBUG
CXXFLAGS  += -g -O0
endif

all: $(LIBPATH)/lib$(LIBNAME).a

clean:
	rm -f $(OBJS) lib$(LIBNAME).a
	rm -rf ../lib

lib$(LIBNAME).a: $(OBJS)
	ar -cvr lib$(LIBNAME).a $(OBJS)
	ranlib lib$(LIBNAME).a

$(LIBPATH):
	mkdir -p ../lib

$(LIBPATH)/lib$(LIBNAME).a: lib$(LIBNAME).a $(LIBPATH)
	cp -d lib$(LIBNAME).a ../lib
