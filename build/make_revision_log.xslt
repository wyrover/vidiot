<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2005/xpath-functions">
    <xsl:output method="text" omit-xml-declaration="yes" indent="no" />
    <xsl:template match="log">
        <xsl:text>Vidiot version history&#xa;&#xa;Trunk (pending)</xsl:text>
        <xsl:for-each select="logentry">
            <xsl:sort select="position()" data-type="number" order="descending" />
            <xsl:if test="@revision>1220">
                <xsl:choose>
                    <xsl:when test="@revision=1234"><xsl:text>&#xa;&#xa;Release 0.1.1 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)</xsl:text></xsl:when>
                    <xsl:when test="@revision=1223"><xsl:text>&#xa;&#xa;Release 0.1.0 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;* Initial version</xsl:text></xsl:when>
                </xsl:choose>
                <xsl:if test="fn:string-length(msg) > 0">
                    <xsl:if test="not(fn:matches(msg, '^\s*$'))">
                        <xsl:text>&#xa;* </xsl:text>
                        <xsl:value-of select="translate(translate(substring(msg, 1, 180), '&#10;' , ' '), '&#13;', '')" />
                        <xsl:text> (r</xsl:text>
                        <xsl:value-of select="@revision" />
                        <xsl:text>)</xsl:text>
                    </xsl:if>
                </xsl:if>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>