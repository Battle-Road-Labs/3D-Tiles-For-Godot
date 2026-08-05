#include "TileMetadata.h"

void TileMetadata::init(size_t tableCount) {
	this->m_tables.reserve(tableCount);
}

const Dictionary& TileMetadata::get_table(int32_t index) const {
	if (this->m_tables.size() == 0) return this->m_empty;
	if ((index + 1) > this->m_tables.size()) return this->m_empty;

	return this->m_tables.at(index);
}


int32_t TileMetadata::get_table_count() const {
	return this->m_tables.size();
}

void TileMetadata::add_table(const CesiumGltf::PropertyTableView& tableView) {
	CesiumPropertyTable_t table{};

	tableView.forEachProperty([this, table](const std::string& propertyId, auto propertyValue) mutable {
		//printf("Property name: %s ", propertyId.c_str());
		table[propertyId.c_str()] = this->make_metadata_value(propertyValue);
	});
	
    this->m_tables.push_back(table);
}

