
file(GLOB font_files fonts/*.ttf)
file(GLOB model_files models/*.obj models/*.mtl)
file(GLOB shader_files shaders/*.vert shaders/*.frag shaders/*.comp shaders/*.rgen shaders/*.rchit shaders/*.rint shaders/*.rmiss)
# file(GLOB shader_files_mesh shaders/meshshader/*.vert shaders/meshshader/*.frag shaders/meshshader/*.mesh shaders/meshshader/*.task)
# list(APPEND shader_files ${shader_files_mesh})
file(GLOB texture_files textures/*.jpg textures/*.png textures/*.txt)

file(GLOB shader_extra_files shaders/*.glsl)
set_source_files_properties(${shader_extra_files} PROPERTIES HEADER_FILE_ONLY TRUE)

# Shader compilation
foreach(shader ${shader_files})
	get_filename_component(FILENAME ${shader} NAME)
	set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
	add_custom_command(OUTPUT ${output_dir}/${FILENAME}.spv
	    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} ${shader} -V
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader} -o ${output_dir}/${FILENAME}.spv
		DEPENDS ${shader}
		COMMENT "Compiling ${FILENAME}")
	list(APPEND SPV_SHADERS ${output_dir}/${FILENAME}.spv)	# get_filename_component(file_name ${shader} NAME)
	message("SHADER: ${shader} -> ${output_dir}/${FILENAME}.spv")
endforeach()

macro(copy_assets asset_files dir_name copied_files)
	message("Copy ${asset_files} to ${dir_name}")
	foreach(asset ${asset_files})
		message("asset: ${asset}")
		get_filename_component(file_name ${asset} NAME)
		get_filename_component(full_path ${asset} ABSOLUTE)
		set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/${dir_name})
		file(MAKE_DIRECTORY ${output_dir})
		set(output_file ${output_dir}/${file_name})
		set(${copied_files} ${copied_files} ${output_file})
		set(${copied_files} ${copied_files} PARENT_SCOPE)
		set_source_files_properties(${asset} PROPERTIES HEADER_FILE_ONLY TRUE)
		configure_file(${full_path} ${output_file} COPYONLY)
	endforeach()
endmacro()

#copy_assets(font_files fonts copied_fonts)
#copy_assets(model_files models copied_models)
copy_assets(${texture_files} textures copied_textures)

source_group("Fonts" FILES ${font_files})
source_group("Models" FILES ${model_files})
source_group("Shaders" FILES ${shader_files} ${shader_extra_files})
source_group("Textures" FILES ${texture_files})

add_custom_target(
	Assets
	DEPENDS ${copied_fonts} ${copied_models} ${compiled_shaders} ${copied_textures}
	SOURCES ${font_files} ${model_files} ${shader_files} ${shader_extra_files} ${texture_files})

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
