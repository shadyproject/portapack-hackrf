/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "app_ais.hpp"

#include "portapack.hpp"
using namespace portapack;

AISModel::AISModel() {
	receiver_model.set_baseband_configuration({
		.mode = 3,
		.sampling_rate = 2457600,
		.decimation_factor = 4,
	});
	receiver_model.set_baseband_bandwidth(1750000);

	log_file.open_for_append("ais.txt");
}

baseband::ais::decoded_packet AISModel::on_packet(const AISPacketMessage& message) {
	const auto result = baseband::ais::packet_decode(message.packet.payload, message.packet.bits_received);

	if( log_file.is_ready() ) {
		std::string entry = result.first + "/" + result.second + "\r\n";
		log_file.write(entry);
	}

	return result;
}	

namespace ui {

void AISView::on_show() {
	Console::on_show();

	auto& message_map = context().message_map();
	message_map.register_handler(Message::ID::AISPacket,
		[this](Message* const p) {
			const auto message = static_cast<const AISPacketMessage*>(p);
			this->log(this->model.on_packet(*message));
		}
	);
}

void AISView::on_hide() {
	auto& message_map = context().message_map();
	message_map.unregister_handler(Message::ID::AISPacket);

	Console::on_hide();
}

void AISView::log(const baseband::ais::decoded_packet decoded) {
	if( decoded.first == "OK" ) {
		writeln(decoded.second);
	}
}

} /* namespace ui */