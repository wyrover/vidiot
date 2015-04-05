# Copyright 2015 Eric Raijmakers.
#
# This file is part of Vidiot.
#
# Vidiot is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vidiot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vidiot. If not, see <http://www.gnu.org/licenses />.

import urllib2, json, sys

# Params for pyscripter: C:\Vidiot\Build\problems.htm Windows
print "Generating knownproblems file"

try:

    assert len(sys.argv) > 2, "Provide output path and platform"
    target_platform = sys.argv[2]

    items = json.loads(urllib2.urlopen("https://sourceforge.net/rest/p/vidiot/tickets/search/?q=!status%3Awont-fix+%26%26+!status%3Aclosed+%26%26+_type%3Abug").read())

    output = open(sys.argv[1],"w")

    output.write( """
    <html>
    <head>
    <title>Vidiot known problems</title>
    </head>
    <body>
    <h1>Vidiot known problems</h1>
    <table>
    <tr><td><ul>
    """)

    for ticket in items["tickets"]:
        ticket_num = ticket[u'ticket_num']
        summary = ticket[u'summary'].rstrip('.')

        print "Ticket #%d: %s" % (ticket_num, summary)

        assert u'custom_fields' in ticket, "Missing custom_fields in #%d %s " % (ticket_num, ticket)
        custom_fields = ticket[u'custom_fields']

        assert u'_platform' in custom_fields, "Missing platform in #%d %s " % (ticket_num, ticket)
        platform = custom_fields[u'_platform']
        assert platform in [ u'All', u'Windows', u'Linux'], "Wrong platform value in #%d %s " % (ticket_num, ticket)

        assert u'_type' in custom_fields, "Missing type in #%d %s " % (ticket_num, ticket)
        type = custom_fields[u'_type']
        assert type in [ u'bug', u'addition'], "Illegal type value in #%d %s " % (ticket_num, ticket)

        ticket_output ="<li>"
        if platform in [ 'All', target_platform ]:
            ticket_output += summary
            ticket_output += " (#" + str(ticket_num) + ")."
            output.write(ticket_output + '</li>\n')

    output.write("""
    </ul></td></tr>
    </table>
    </body>
    </html>
    """)

    output.close()
except Exception, e:
    print e
    input("Generating known problems file failed. \nPress Enter to continue...")