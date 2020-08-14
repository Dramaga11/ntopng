--
-- (C) 2019-20 - ntop.org
--

-- Companion scripts (in addition to i18n)
-- scripts/callbacks/status_defs/status_udp_unidirectional.lua
-- scripts/callbacks/interface/flow/udp.lua

local flow_consts = require("flow_consts")
local user_scripts = require("user_scripts")

-- #################################################################

local script = {
   -- Script category
   category = user_scripts.script_categories.network,

   l4_proto = "udp",

   -- NOTE: hooks defined below
   hooks = {},

   gui = {
      i18n_title = "flow_callbacks_config.udp_unidirectional",
      i18n_description = "flow_callbacks_config.udp_unidirectional_description",
   }
}

-- #################################################################

local function unidirectionalProtoWhitelist(proto_id)
   if((proto_id == 17)        -- Syslog
	 or (proto_id == 18)  -- DHCP
      	 or (proto_id == 87)  -- RTP
	 or (proto_id == 103) -- DHCPV6
	 or (proto_id == 128) -- NetFlow
      	 or (proto_id == 129) -- sFlow
   ) then
      return(true)
   end
   
   return(false) -- Not whitelisted
end

-- #################################################################

function script.hooks.all(now)
   if((flow.getPacketsRcvd() == 0) and (flow.getPacketsSent() > 0)) then
      -- Now check if the recipient isn't a broadcast/multicast address
      if(flow.isServerUnicast() and not(unidirectionalProtoWhitelist(flow.getnDPIAppProtoId()))) then
         flow.setStatus(flow_consts.status_types.status_udp_unidirectional, 5--[[ flow score]], 5--[[ cli score ]], 1--[[ srv score ]])
      end
   else
      flow.clearStatus(flow_consts.status_types.status_udp_unidirectional)
   end
end

-- #################################################################

return script
