<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2005/xpath-functions">
<xsl:output method="text" omit-xml-declaration="yes" indent="no" />

<!--
// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses />.
-->

<xsl:template match="log">

<![CDATA[
<html>
<head>
<title>Vidiot version history</title>
</head>
<body>
<h1>Vidiot version history</h1>
<table>
<tr><td><ul>
]]>

  <xsl:for-each select="logentry">
    <xsl:sort select="position()" data-type="number" order="descending" />
    <xsl:variable name="revision" select="@revision" />
    <xsl:if test="$revision>1220">
      <xsl:choose>
        <xsl:when test="$revision=1820"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.8  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1818"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.7  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1773"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.6  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1723"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.5  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1682"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.4  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1674"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.3  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1670"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.2  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1647"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.3.1  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1613"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.10 (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1608"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.9  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1583"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.8  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1556"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.7  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1453"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.6  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1415"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.5  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1340"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.4  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1322"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.3  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1313"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.2  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1310"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.2.1  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1267"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.1.3  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1246"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.1.2  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1235"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.1.1  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul>]]></xsl:when>
        <xsl:when test="$revision=1223"><![CDATA[</ul></td></tr><tr><td><h2>]]>Release 0.1.0  (revision <xsl:value-of select="$revision" />)<![CDATA[</h2></td></tr><tr><td><ul><li>Initial version</li>]]></xsl:when>
      </xsl:choose>
      
      <xsl:if test="fn:string-length(msg) > 0">
        <xsl:for-each select="tokenize(msg,'\n')">
          <xsl:variable name="entry" select="." />
          <xsl:if test="not(fn:matches($entry, '^\s*$'))">
              <xsl:variable name="revisionlabel" select="concat( '(r' , $revision , ')' )" />
              <xsl:variable name="msg_and_label" select="concat( '' , $entry , ' ' , $revisionlabel )" />
              <xsl:variable name="removed_newlines" select="translate($msg_and_label, '&#10;&#13;' , ' ')" />
              <xsl:variable name="word_wrapped_long_lines" select="replace(concat(normalize-space( $removed_newlines ) , ' '),      '(.{0,80}) ', '$1&#xa;  ')" />
              <xsl:variable name="remove_last_indent" select="substring($word_wrapped_long_lines, 1, string-length($word_wrapped_long_lines) - string-length('  '))" />
              <![CDATA[<li>]]><xsl:value-of select="$remove_last_indent" /><![CDATA[</li>]]>
          </xsl:if>
        </xsl:for-each>
      </xsl:if>
    </xsl:if>
  </xsl:for-each>
  
<![CDATA[
</ul></td></tr>
</table>
</body>
</html>
]]>
  
</xsl:template>

</xsl:stylesheet>