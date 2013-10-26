<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2005/xpath-functions">
    <xsl:output method="text" omit-xml-declaration="yes" indent="no" />
    <xsl:template match="log">
        <xsl:text>Vidiot version history&#xa;&#xa;Trunk (pending)&#xa;</xsl:text>
        <xsl:for-each select="logentry">
            <xsl:sort select="position()" data-type="number" order="descending" />
            <xsl:if test="@revision>1220">
                <xsl:choose>
                    <xsl:when test="@revision=1313"><xsl:text>&#xa;Release 0.2.2 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;</xsl:text></xsl:when>
                    <xsl:when test="@revision=1310"><xsl:text>&#xa;Release 0.2.1 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;</xsl:text></xsl:when>
                    <xsl:when test="@revision=1267"><xsl:text>&#xa;Release 0.1.3 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;</xsl:text></xsl:when>
                    <xsl:when test="@revision=1246"><xsl:text>&#xa;Release 0.1.2 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;</xsl:text></xsl:when>
                    <xsl:when test="@revision=1235"><xsl:text>&#xa;Release 0.1.1 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;</xsl:text></xsl:when>
                    <xsl:when test="@revision=1223"><xsl:text>&#xa;Release 0.1.0 (revision </xsl:text><xsl:value-of select="@revision"/><xsl:text>)&#xa;* Initial version</xsl:text></xsl:when>
                </xsl:choose>
                <xsl:if test="fn:string-length(msg) > 0">
                    <xsl:if test="not(fn:matches(msg, '^\s*$'))">
                        <xsl:variable name="revisionlabel" select="concat( '(r' , @revision , ')' )"/>
                        <xsl:variable name="msg_and_label" select="concat( '* ' , msg , ' ' , $revisionlabel )"/>
                        <xsl:variable name="removed_newlines" select="translate($msg_and_label, '&#10;&#13;' , ' ')"/>
                        <xsl:variable name="word_wrapped_long_lines" select="replace(concat(normalize-space( $removed_newlines ) , ' '),      '(.{0,80}) ', '$1&#xa;  ')"/>
                        <xsl:variable name="remove_last_indent" select="substring($word_wrapped_long_lines, 1, string-length($word_wrapped_long_lines) - string-length('  '))"/>
                        <xsl:value-of select="$remove_last_indent" />
                    </xsl:if>
                </xsl:if>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>